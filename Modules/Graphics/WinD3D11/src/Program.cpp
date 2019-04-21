#include "Program.h"
#include "Graphics.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "base/String.h"
#include "utils/hash/CRC.h"
#include <vector>

namespace aurora::modules::graphics::win_d3d11 {
	Program::InLayout::~InLayout() {
		delete[] formats;
		if (layout) layout->Release();
	}

	bool Program::InLayout::isEqual(const D3D11_INPUT_ELEMENT_DESC* inElements, ui32 num) const {
		for (ui32 i = 0, n = num; i < n; ++i) {
			if (formats[i] != inElements[i].Format) return false;
		}
		return true;
	}


	void Program::ResourceLayout::clear(Graphics& g) {
		for (auto& buffer : constantBuffers) g.unregisterConstantLayout(buffer);
		constantBuffers.clear();
	}


	Program::Program(Graphics& graphics) : IProgram(graphics),
		_vertBlob(nullptr),
		_vs(nullptr),
		_ps(nullptr),
		_inElements(nullptr),
		_numInElements(0),
		_curInLayout(nullptr) {
	}

	Program::~Program() {
		_release();
	}

	bool Program::upload(const ProgramSource& vert, const ProgramSource& frag) {
		_release();

		DXObjGuard objs;

		auto g = (Graphics*)_graphics;

		auto& sm = g->getSupportShaderModel();

		_vertBlob = _compileShader(vert, ProgramSource::toHLSLShaderModel(ProgramStage::VS, vert.version.empty() ? sm : vert.version).c_str());
		if (!_vertBlob) return false;

		auto pixelBlob = _compileShader(frag, ProgramSource::toHLSLShaderModel(ProgramStage::PS, frag.version.empty() ? sm : frag.version).c_str());
		if (!pixelBlob) {
			_release();
			return false;
		}
		objs.add(pixelBlob);

		auto device = g->getDevice();

		HRESULT hr = device->CreateVertexShader(_vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), 0, &_vs);
		if (FAILED(hr)) {
			_release();
			return false;
		}

		ID3D11ShaderReflection* vsr = nullptr;
		if (FAILED(D3DReflect(_vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&vsr))) {
			_release();
			return false;
		}
		objs.add(vsr);

		D3D11_SHADER_DESC sDesc;
		vsr->GetDesc(&sDesc);
		
		_parseInLayout(sDesc, *vsr);

		_curInLayout = _getOrCreateInputLayout();

		_parseResourceLayout(sDesc, *vsr, _vsResLayout);

		ID3D11PixelShader* fs = nullptr;
		if (FAILED(device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &_ps))) {
			_release();
			return false;
		}

		ID3D11ShaderReflection* psr = nullptr;
		if (FAILED(D3DReflect(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&psr))) {
			_release();
			return false;
		}
		objs.add(psr);

		psr->GetDesc(&sDesc);

		_parseResourceLayout(sDesc, *psr, _psResLayout);

		_calcConstantLayoutSameBuffers(std::vector<std::vector<ConstantBufferLayout>*>({ &_vsResLayout.constantBuffers, &_psResLayout.constantBuffers }));

		return true;
	}

	bool Program::use() {
		if (_vs) {
			auto context = ((Graphics*)_graphics)->getContext();

			context->VSSetShader(_vs, 0, 0);
			context->PSSetShader(_ps, 0, 0);

			return true;
		}
		return false;
	}

	ConstantBuffer* Program::_getConstantBuffer(const ConstantBufferLayout& constantLayout, const ConstantFactory& factory) {
		if (constantLayout.sameId) {
			auto cb = _usingSameBuffers[constantLayout.sameId - 1];
			if (cb) return cb;
		}

		ui32 exclusiveCount = 0, autoCount = 0, unknownCount = 0;
		for (auto& var : constantLayout.vars) {
			auto c = factory.get(var.name);
			if (c) {
				if (c->getUsage() == ConstantUsage::EXCLUSIVE) {
					++exclusiveCount;
				} else if (c->getUsage() == ConstantUsage::AUTO) {
					++autoCount;
				}
			} else {
				++unknownCount;
			}
			_tempConstants.emplace_back(c);
		}

		ui32 numVars = constantLayout.vars.size();
		if (unknownCount >= numVars) return nullptr;

		ConstantBuffer* cb;
		if (exclusiveCount > 0 && exclusiveCount + autoCount == numVars) {
			auto g = (Graphics*)_graphics;
			cb = g->getExclusiveConstantBuffer(_tempConstants, constantLayout);

			if (cb) {
				if (g->getFeatureOptions().MapNoOverwriteOnDynamicConstantBuffer) {
					bool isMaping = false;
					for (ui32 i = 0; i < numVars; ++i) {
						auto c = _tempConstants[i];
						if (c && c->getUpdateId() != cb->recordUpdateIds[i]) {
							auto& var = constantLayout.vars[i];

							if (!isMaping) {
								if (!cb->map(BufferUsage::CPU_WRITE)) break;
								isMaping = true;
							}
							cb->recordUpdateIds[i] = c->getUpdateId();
							_updateConstantBuffer(cb, *c, var);
						}
					}

					if (isMaping) cb->unmap();
				} else {
					bool needUpdate = false;
					for (ui32 i = 0; i < numVars; ++i) {
						auto c = _tempConstants[i];
						if (c && c->getUpdateId() != cb->recordUpdateIds[i]) {
							needUpdate = true;
							cb->recordUpdateIds[i] = c->getUpdateId();
						}
					}
					if (needUpdate) _constantBufferUpdateAll(cb, constantLayout.vars);
				}
			}
		} else {
			cb = ((Graphics*)_graphics)->popShareConstantBuffer(constantLayout.size);
			if (cb) _constantBufferUpdateAll(cb, constantLayout.vars);
		}

		_tempConstants.clear();
		if (cb && constantLayout.sameId) _usingSameBuffers[constantLayout.sameId - 1] = cb;

		return cb;
	}

	void Program::_updateConstantBuffer(ConstantBuffer* cb, const Constant& c, const ConstantBufferLayout::Var& var) {
		ui32 size = c.getSize();
		if (!size) return;

		ui16 pes = c.getPerElementSize();
		if (pes < size) {
			auto remainder = pes & 0b1111;
			auto offset = remainder ? 16 - remainder : 0;
			if (offset) {
				auto max = std::min<ui32>(size, var.size);
				ui32 cur = 0, cbOffset = var.offset;
				auto data = c.getData();
				do {
					cb->write(cbOffset, data, pes);
					cbOffset += pes + offset;
					cur += pes;
				} while (cur < max);
			} else {
				cb->write(var.offset, c.getData(), std::min<ui32>(size, var.size));
			}
		} else {
			cb->write(var.offset, c.getData(), std::min<ui32>(size, var.size));
		}
	}

	void Program::_constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Var>& vars) {
		if (cb->map(BufferUsage::CPU_WRITE)) {
			for (ui32 i = 0, n = _tempConstants.size(); i < n; ++i) {
				auto c = _tempConstants[i];
				if (c) _updateConstantBuffer(cb, *c, vars[i]);
			}

			cb->unmap();
		}
	}

	void Program::draw(const VertexBufferFactory* vertexFactory, const ConstantFactory* constantFactory,
		const IIndexBuffer* indexBuffer, ui32 count, ui32 offset) {
		if (_vs && vertexFactory && indexBuffer, count > 0) {
			auto g = (Graphics*)_graphics;
			auto context = g->getContext();

			bool inElementsDirty = false;
			for (ui32 i = 0, n = _inVerBufInfos.size(); i < n; ++i) {
				auto& info = _inVerBufInfos[i];
				auto vb = (VertexBuffer*)vertexFactory->get(info.name);
				if (vb) {
					auto& ie = _inElements[i];
					DXGI_FORMAT fmt;
					if (vb->use(info.slot, fmt)) {
						if (ie.Format != fmt) {
							inElementsDirty = true;
							ie.Format = fmt;
						}
					}
				}
			}

			if (inElementsDirty) _curInLayout = _getOrCreateInputLayout();

			if (_curInLayout) {
				if (constantFactory) {
					_useConstants<ProgramStage::VS>(_vsResLayout, *constantFactory);
					_useConstants<ProgramStage::PS>(_psResLayout, *constantFactory);
				}

				context->IASetInputLayout(_curInLayout);
				((IndexBuffer*)indexBuffer)->draw(count, offset);

				if (constantFactory) {
					for (ui32 i = 0, n = _usingSameBuffers.size(); i < n; ++i) _usingSameBuffers[i] = nullptr;
				}
				((Graphics*)_graphics)->resetUsedShareConstantBuffers();
			}
		}
	}

	void Program::_release() {
		_curInLayout = nullptr;
		_inVerBufInfos.clear();
		_inLayouts.clear();

		if (_numInElements > 0) {
			for (ui32 i = 0; i < _numInElements; ++i) delete[] _inElements[i].SemanticName;
			delete[] _inElements;
			_numInElements = 0;
		}

		if (_curInLayout) {
			_curInLayout->Release();
			_curInLayout = nullptr;
		}

		if (_vs) {
			_vs->Release();
			_vs = nullptr;
		}

		if (_ps) {
			_ps->Release();
			_ps = nullptr;
		}

		if (_vertBlob) {
			_vertBlob->Release();
			_vertBlob = nullptr;
		}

		auto& g = *(Graphics*)_graphics;
		_vsResLayout.clear(g);
		_psResLayout.clear(g);
		_usingSameBuffers.clear();
	}

	ID3DBlob* Program::_compileShader(const ProgramSource& source, const i8* target) {
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef AE_DEBUG
		shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		ID3DBlob* buffer = nullptr, *errorBuffer = nullptr;
		HRESULT hr = D3DCompile(source.data.getBytes(), source.data.getLength(), nullptr, nullptr, nullptr, 
			ProgramSource::getEntryPoint(source).c_str(), target, shaderFlags, 0, &buffer, &errorBuffer);

		if (FAILED(hr)) {
			if (errorBuffer) {
				println("d3d11 compile shader err : %s", errorBuffer->GetBufferPointer());
				errorBuffer->Release();
			}

			return nullptr;
		} else if (errorBuffer) {
			errorBuffer->Release();
		}

		return buffer;
	}

	ID3D11InputLayout* Program::_getOrCreateInputLayout() {
		for (ui32 i = 0, n = _inLayouts.size(); i < n; ++i) {
			auto& il = _inLayouts[i];
			if (il.isEqual(_inElements, _numInElements)) return il.layout;
		}

		auto& il = _inLayouts.emplace_back();
		il.formats = new ui32[_numInElements];
		for (ui32 i = 0; i < _numInElements; ++i) il.formats[i] = _inElements[i].Format;
		auto hr = ((Graphics*)_graphics)->getDevice()->CreateInputLayout(_inElements, _numInElements, _vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), &il.layout);
		return il.layout;
	}

	void Program::_parseInLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref) {
		_numInElements = desc.InputParameters;
		if (_numInElements > 0) {
			ui32 offset = 0;
			_inElements = new D3D11_INPUT_ELEMENT_DESC[desc.InputParameters];
			D3D11_SIGNATURE_PARAMETER_DESC pDesc;
			for (ui32 i = 0; i < _numInElements; ++i) {
				ref.GetInputParameterDesc(i, &pDesc);

				auto& ieDesc = _inElements[i];
				ui32 len = strlen(pDesc.SemanticName);
				i8* name = new i8[len + 1];
				ieDesc.SemanticName = name;
				name[len] = 0;
				memcpy(name, pDesc.SemanticName, len);
				//ieDesc.SemanticName = spDesc.SemanticName;
				ieDesc.SemanticIndex = pDesc.SemanticIndex;
				ieDesc.InputSlot = i;
				ieDesc.AlignedByteOffset = 0;
				ieDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				ieDesc.InstanceDataStepRate = 0;

				auto& info = _inVerBufInfos.emplace_back();
				info.name = pDesc.SemanticName + String::toString(pDesc.SemanticIndex);
				info.slot = i;

				if (pDesc.Mask == 1) {
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32_UINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32_SINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32_FLOAT;
					}
					offset += 4;
				} else if (pDesc.Mask <= 3) {
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_UINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_SINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
					}
					offset += 8;
				} else if (pDesc.Mask <= 7) {
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
					}
					offset += 12;
				} else if (pDesc.Mask <= 15) {
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					}
					offset += 16;
				}

				if (i == 0) {
					ieDesc.Format = DXGI_FORMAT_R8G8_UINT;
				}
			}
		}
	}

	void Program::_parseResourceLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, ResourceLayout& dst) {
		D3D11_SHADER_INPUT_BIND_DESC ibDesc;
		for (ui32 i = 0; i < desc.BoundResources; ++i) {
			ref.GetResourceBindingDesc(i, &ibDesc);

			switch (ibDesc.Type) {
			case D3D_SIT_CBUFFER:
			{
				auto& buffer = dst.constantBuffers.emplace_back();
				buffer.name = ibDesc.Name;
				buffer.bindPoint = ibDesc.BindPoint;
				buffer.size = 0;
				buffer.sameId = 0;

				break;
			}
			case D3D_SIT_TEXTURE:
			{
				auto& tex = dst.textures.emplace_back();
				tex.name = ibDesc.Name;
				tex.bindPoint = ibDesc.BindPoint;

				break;
			}
			case D3D_SIT_SAMPLER:
			{
				break;
			}
			default:
				break;
			}
		}

		D3D11_SHADER_BUFFER_DESC bDesc;
		D3D11_SHADER_VARIABLE_DESC vDesc;
		for (ui32 i = 0; i < desc.ConstantBuffers; ++i) {
			auto cb = ref.GetConstantBufferByIndex(i);
			cb->GetDesc(&bDesc);

			ConstantBufferLayout* buffer = nullptr;
			i16 idx = -1;
			for (i16 j = 0, n = dst.constantBuffers.size(); j < n;  ++j) {
				if (String::isEqual(dst.constantBuffers[j].name.c_str(), bDesc.Name)) {
					idx = j;
					buffer = &dst.constantBuffers[j];
					break;
				}
			}

			if (buffer) {
				i8 end = 0;
				buffer->featureCode = hash::CRC::CRC64StreamBegin();
				hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&bDesc.Variables, sizeof(bDesc.Variables));

				for (ui32 j = 0; j < bDesc.Variables; ++j) {
					auto var = cb->GetVariableByIndex(j);
					var->GetDesc(&vDesc);

					auto& v = buffer->vars.emplace_back();
					v.name = vDesc.Name;
					v.offset = vDesc.StartOffset;
					v.size = vDesc.Size;
					buffer->size += v.size;

					hash::CRC::CRC64StreamIteration(buffer->featureCode, v.name.c_str(), v.name.size());
					hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&end, sizeof(end));
					hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&v.offset, sizeof(v.offset));
					hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&v.size, sizeof(v.size));
				}

				auto& v = buffer->vars[bDesc.Variables - 1];
				auto len = v.offset + v.size;
				auto remainder = len & 0b1111;
				buffer->size = remainder ? len + 16 - remainder : len;

				hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&buffer->size, sizeof(buffer->size));
				hash::CRC::CRC64StreamEnd(buffer->featureCode);

				((Graphics*)_graphics)->registerConstantLayout(*buffer);
			}
		}
	}

	void Program::_calcConstantLayoutSameBuffers(std::vector<std::vector<ConstantBufferLayout>*>& constBufferLayouts) {
		ui32 sameId = 0;
		i32 n = (i32)constBufferLayouts.size();
		for (i32 i = 0; i < n; ++i) {
			auto buffers0 = constBufferLayouts[i];
			for (i32 j = 0; j < n; ++j) {
				if (i == j) continue;

				auto buffers1 = constBufferLayouts[j];
				
				for (auto& buffer0 : *buffers0) {
					for (auto& buffer1 : *buffers1) {
						if (buffer0.featureCode == buffer1.featureCode) {
							if (buffer0.sameId == 0) {
								buffer0.sameId = ++sameId;
								_usingSameBuffers.emplace_back(nullptr);
							}
							buffer1.sameId = buffer0.sameId;
							break;
						}
					}
				}
			}
		}
	}
}