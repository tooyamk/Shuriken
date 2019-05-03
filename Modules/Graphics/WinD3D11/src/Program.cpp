#include "Program.h"
#include "Graphics.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "base/String.h"
#include "utils/hash/CRC.h"
#include <vector>

namespace aurora::modules::graphics::win_d3d11 {
	Program::InLayout::InLayout(ui32 numInElements) :
		formats(numInElements) {
	}

	Program::InLayout::~InLayout() {
		if (layout) layout->Release();
	}

	bool Program::InLayout::isEqual(const D3D11_INPUT_ELEMENT_DESC* inElements, ui32 num) const {
		for (ui32 i = 0, n = num; i < n; ++i) {
			if (formats[i] != inElements[i].Format) return false;
		}
		return true;
	}


	void Program::ParameterLayout::clear(Graphics& g) {
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

		auto g = _graphics.get<Graphics>();

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

		_parseParameterLayout(sDesc, *vsr, _vsParamLayout);

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

		_parseParameterLayout(sDesc, *psr, _psParamLayout);

		_calcConstantLayoutSameBuffers(std::vector<std::vector<ConstantBufferLayout>*>({ &_vsParamLayout.constantBuffers, &_psParamLayout.constantBuffers }));

		return true;
	}

	bool Program::use() {
		if (_vs) {
			auto context = _graphics.get<Graphics>()->getContext();

			context->VSSetShader(_vs, 0, 0);
			context->PSSetShader(_ps, 0, 0);

			return true;
		}
		return false;
	}

	ConstantBuffer* Program::_getConstantBuffer(const ConstantBufferLayout& cbLayout, const ShaderParameterFactory& factory) {
		if (cbLayout.sameId) {
			auto cb = _usingSameConstBuffers[cbLayout.sameId - 1];
			if (cb) return cb;
		}

		ui32 exclusiveCount = 0, autoCount = 0, unknownCount = 0;
		for (auto& var : cbLayout.vars) {
			auto c = factory.get(var.name);
			if (c) {
				if (c->getUsage() == ShaderParameterUsage::EXCLUSIVE) {
					++exclusiveCount;
				} else if (c->getUsage() == ShaderParameterUsage::AUTO) {
					++autoCount;
				}
			} else {
				++unknownCount;
			}
			_tempParams.emplace_back(c);
		}

		ui32 numVars = cbLayout.vars.size();
		if (unknownCount >= numVars) return nullptr;

		ConstantBuffer* cb;
		if (exclusiveCount > 0 && exclusiveCount + autoCount == numVars) {
			auto g = _graphics.get<Graphics>();
			cb = g->getExclusiveConstantBuffer(_tempParams, cbLayout);

			if (cb) {
				if (g->getInternalFeatures().MapNoOverwriteOnDynamicConstantBuffer) {
					bool isMaping = false;
					for (ui32 i = 0; i < numVars; ++i) {
						auto param = _tempParams[i];
						if (param && param->getUpdateId() != cb->recordUpdateIds[i]) {
							auto& var = cbLayout.vars[i];

							if (!isMaping) {
								if (cb->map(Usage::CPU_WRITE) == Usage::NONE) break;
								isMaping = true;
							}
							cb->recordUpdateIds[i] = param->getUpdateId();
							_updateConstantBuffer(cb, *param, var);
						}
					}

					if (isMaping) cb->unmap();
				} else {
					bool needUpdate = false;
					for (ui32 i = 0; i < numVars; ++i) {
						auto c = _tempParams[i];
						if (c && c->getUpdateId() != cb->recordUpdateIds[i]) {
							needUpdate = true;
							cb->recordUpdateIds[i] = c->getUpdateId();
						}
					}
					if (needUpdate) _constantBufferUpdateAll(cb, cbLayout.vars);
				}
			}
		} else {
			cb = _graphics.get<Graphics>()->popShareConstantBuffer(cbLayout.size);
			if (cb) _constantBufferUpdateAll(cb, cbLayout.vars);
		}

		_tempParams.clear();
		if (cb && cbLayout.sameId) _usingSameConstBuffers[cbLayout.sameId - 1] = cb;

		return cb;
	}

	void Program::_updateConstantBuffer(ConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Var& var) {
		ui32 size = param.getSize();
		if (!size) return;

		ui16 pes = param.getPerElementSize();
		if (pes < size) {
			auto remainder = pes & 0b1111;
			if (remainder) {
				auto offset = pes + 16 - remainder;
				auto max = std::min<ui32>(size, var.size);
				ui32 cur = 0, cbOffset = var.offset;
				auto data = param.getData();
				do {
					cb->write(cbOffset, data, pes);
					cbOffset += offset;
					cur += pes;
				} while (cur < max);
			} else {
				cb->write(var.offset, param.getData(), std::min<ui32>(size, var.size));
			}
		} else {
			cb->write(var.offset, param.getData(), std::min<ui32>(size, var.size));
		}
	}

	void Program::_constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Var>& vars) {
		if (cb->map(Usage::CPU_WRITE) != Usage::NONE) {
			for (ui32 i = 0, n = _tempParams.size(); i < n; ++i) {
				auto param = _tempParams[i];
				if (param) _updateConstantBuffer(cb, *param, vars[i]);
			}

			cb->unmap();
		}
	}

	void Program::draw(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory,
		const IIndexBuffer* indexBuffer, ui32 count, ui32 offset) {
		if (_vs && vertexFactory && indexBuffer && _graphics == indexBuffer->getGraphics() && count > 0) {
			auto g = _graphics.get<Graphics>();
			auto context = g->getContext();

			bool inElementsDirty = false;
			for (ui32 i = 0, n = _inVerBufInfos.size(); i < n; ++i) {
				auto& info = _inVerBufInfos[i];
				auto vb = vertexFactory->get(info.name);
				if (vb && _graphics == vb->getGraphics()) {
					auto& ie = _inElements[i];
					DXGI_FORMAT fmt;
					if (((VertexBuffer*)vb)->use(info.slot, fmt)) {
						if (ie.Format != fmt) {
							inElementsDirty = true;
							ie.Format = fmt;
						}
					}
				}
			}

			if (inElementsDirty) _curInLayout = _getOrCreateInputLayout();

			if (_curInLayout) {
				if (paramFactory) {
					_useParameters<ProgramStage::VS>(_vsParamLayout, *paramFactory);
					_useParameters<ProgramStage::PS>(_psParamLayout, *paramFactory);
				}

				context->IASetInputLayout(_curInLayout);
				((IndexBuffer*)indexBuffer)->draw(count, offset);

				if (paramFactory) {
					for (ui32 i = 0, n = _usingSameConstBuffers.size(); i < n; ++i) _usingSameConstBuffers[i] = nullptr;
				}
				g->resetUsedShareConstantBuffers();
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

		auto& g = *_graphics.get<Graphics>();
		_vsParamLayout.clear(g);
		_psParamLayout.clear(g);
		_usingSameConstBuffers.clear();
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

		auto& il = _inLayouts.emplace_back(_numInElements);
		for (ui32 i = 0; i < _numInElements; ++i) il.formats[i] = _inElements[i].Format;
		auto hr = _graphics.get<Graphics>()->getDevice()->CreateInputLayout(_inElements, _numInElements, _vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), &il.layout);
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
			}
		}
	}

	void Program::_parseParameterLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, ParameterLayout& dst) {
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
				auto& tex = dst.samplers.emplace_back();
				tex.name = ibDesc.Name;
				tex.bindPoint = ibDesc.BindPoint;

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
					buffer->size = bDesc.Size;
					break;
				}
			}

			if (buffer) {
				i8 end = 0;
				buffer->featureCode = hash::CRC::CRC64StreamBegin();
				hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&buffer->size, sizeof(buffer->size));
				hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&bDesc.Variables, sizeof(bDesc.Variables));

				for (ui32 j = 0; j < bDesc.Variables; ++j) {
					auto var = cb->GetVariableByIndex(j);
					var->GetDesc(&vDesc);

					if (vDesc.uFlags & D3D_SVF_USED) {
						auto& v = buffer->vars.emplace_back();
						v.name = vDesc.Name;
						v.offset = vDesc.StartOffset;
						v.size = vDesc.Size;

						_parseConstantVar(v, var->GetType());

						hash::CRC::CRC64StreamIteration(buffer->featureCode, v.name.c_str(), v.name.size());
						hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&end, sizeof(end));
						hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&v.offset, sizeof(v.offset));
						hash::CRC::CRC64StreamIteration(buffer->featureCode, (i8*)&v.size, sizeof(v.size));
					}
				}

				//auto remainder = buffer->size & 0b1111;
				//if (remainder) buffer->size += 16 - remainder;
				
				hash::CRC::CRC64StreamEnd(buffer->featureCode);

				_graphics.get<Graphics>()->registerConstantLayout(*buffer);
			}
		}
	}

	void Program::_parseConstantVar(ConstantBufferLayout::Var& var, ID3D11ShaderReflectionType* type) {
		D3D11_SHADER_TYPE_DESC desc;
		type->GetDesc(&desc);
		var.offset += desc.Offset;
		if (desc.Class == D3D_SVC_STRUCT) {
			var.structMembers.resize(desc.Members);
			for (ui32 i = 0; i < desc.Members; ++i) {
				auto& memberVar = var.structMembers[i];
				memberVar.name = type->GetMemberTypeName(i);
				memberVar.offset = 0;
				memberVar.size = 0;
				
				_parseConstantVar(memberVar, type->GetMemberTypeByIndex(i));
				
				int a = 1;
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
								_usingSameConstBuffers.emplace_back(nullptr);
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