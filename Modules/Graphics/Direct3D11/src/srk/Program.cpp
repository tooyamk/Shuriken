#include "Program.h"
#include "Graphics.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "srk/GraphicsBuffer.h"
#include "srk/Literals.h"
#include "srk/ShaderParameter.h"
#include "srk/StringUtility.h"
#include <vector>

namespace srk::modules::graphics::d3d11 {
	Program::InputLayout::InputLayout(uint32_t numInElements) :
		formats(numInElements),
		layout(nullptr) {
	}

	Program::InputLayout::~InputLayout() {
		if (layout) layout->Release();
	}

	bool Program::InputLayout::equal(const D3D11_INPUT_ELEMENT_DESC* inputElements, uint32_t num) const {
		for (uint32_t i = 0, n = num; i < n; ++i) {
			if (formats[i] != inputElements[i].Format) return false;
		}
		return true;
	}


	void Program::ParameterLayout::clear(Graphics& g) {
		for (auto& buffer : constantBuffers) g.getConstantBufferManager().unregisterConstantLayout(buffer);
		constantBuffers.clear();
		textures.clear();
		samplers.clear();
	}


	Program::MyIncludeHandler::MyIncludeHandler(const ProgramIncludeHandler& handler) :
		_handler(handler) {
	}

	HRESULT Program::MyIncludeHandler::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) {
		if (_handler) {
			ProgramIncludeInfo pii;
			pii.file = pFileName;
			_data = _handler(pii);
			*ppData = _data.getSource();
			*pBytes = _data.getLength();

			return S_OK;
		}

		return E_FAIL;
	}

	HRESULT Program::MyIncludeHandler::Close(LPCVOID pData) {
		_data.dispose();
		return S_OK;
	}


	Program::Program(Graphics& graphics) : IProgram(graphics),
		_vertBlob(nullptr),
		_vs(nullptr),
		_ps(nullptr),
		_inputElements(nullptr),
		_numInElements(0),
		_curInLayout(nullptr) {
	}

	Program::~Program() {
		destroy();
	}

	const void* Program::getNative() const {
		return this;
	}

	bool Program::create(const ProgramSource& vert, const ProgramSource& frag, const ProgramDefine* defines, size_t numDefines, const ProgramIncludeHandler& includeHandler, const ProgramInputHandler& inputHandler, const ProgramTranspileHandler& transpileHandler) {
		destroy();

		DXObjGuard objs;

		auto g = _graphics.get<Graphics>();

		auto& sm = g->getSupportShaderModel();

		std::vector<D3D_SHADER_MACRO> d3dDefines(numDefines + 1);
		{
			for (size_t i = 0; i < numDefines; ++i) {
				auto& d3dDef = d3dDefines[i];
				auto def = defines + (i << 1);

				d3dDef.Name = def->name.data();
				d3dDef.Definition = def->value.data();
			}
			auto& def = d3dDefines[numDefines];
			def.Name = nullptr;
			def.Definition = nullptr;
		}

		_vertBlob = _compileShader(vert, ProgramStage::VS, ProgramSource::toHLSLShaderModel(ProgramStage::VS, vert.version.empty() ? sm : vert.version), d3dDefines.data(), defines, numDefines, includeHandler, transpileHandler);
		if (!_vertBlob) return false;

		auto pixelBlob = _compileShader(frag, ProgramStage::PS, ProgramSource::toHLSLShaderModel(ProgramStage::PS, frag.version.empty() ? sm : frag.version), d3dDefines.data(), defines, numDefines, includeHandler, transpileHandler);
		if (!pixelBlob) {
			destroy();
			return false;
		}
		objs.add(pixelBlob);

		auto device = g->getDevice();

		if (FAILED(device->CreateVertexShader(_vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), nullptr, &_vs))) {
			destroy();
			return false;
		}

		ID3D11ShaderReflection* vsr = nullptr;
		if (FAILED(D3DReflect(_vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&vsr))) {
			destroy();
			return false;
		}
		objs.add(vsr);

		D3D11_SHADER_DESC sDesc;
		vsr->GetDesc(&sDesc);
		
		_parseInputLayout(sDesc, *vsr, inputHandler);

		_curInLayout = _getOrCreateInputLayout();

		_parseParameterLayout(sDesc, *vsr, _vsParamLayout);

		ID3D11PixelShader* fs = nullptr;
		if (FAILED(device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &_ps))) {
			destroy();
			return false;
		}

		ID3D11ShaderReflection* psr = nullptr;
		if (FAILED(D3DReflect(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&psr))) {
			destroy();
			return false;
		}
		objs.add(psr);

		psr->GetDesc(&sDesc);

		_parseParameterLayout(sDesc, *psr, _psParamLayout);

		std::vector<std::vector<MyConstantBufferLayout>*> layouts = { &_vsParamLayout.constantBuffers, &_psParamLayout.constantBuffers };
		_calcConstantLayoutSameBuffers(layouts);

		return true;
	}

	const ProgramInfo& Program::getInfo() const {
		return _info;
	}

	bool Program::use(const IVertexAttributeGetter* vertexAttributeGetter, const IShaderParameterGetter* shaderParamGetter) {
		if (_vs) {
			auto g = _graphics.get<Graphics>();
			g->useShader<ProgramStage::VS>(_vs, nullptr, 0);
			g->useShader<ProgramStage::PS>(_ps, nullptr, 0);

			auto context = g->getContext();

			bool inElementsDirty = false;
			for (size_t i = 0, n = _info.vertices.size(); i < n; ++i) {
				auto& info = _info.vertices[i];
				auto va = vertexAttributeGetter->get(info.name);
				if (!va) continue;

				auto& vb = va->resource;
				if (!vb || _graphics != vb->getGraphics()) continue;

				UINT stride = vb->getStride();
				if (!stride) continue;

				auto native = (BaseBuffer*)vb->getNative();
				if (!native) continue;

				auto buf = (ID3D11Buffer*)native->handle;
				if (!buf) continue;

				auto& desc = va->desc;
				auto fmt = Graphics::convertVertexFormat(desc.format);
				if (fmt == DXGI_FORMAT_UNKNOWN) continue;
				
				UINT offset = desc.offset;
				g->useVertexBuffers(_inVerBufSlots[i], 1, &buf, &stride, &offset);

				auto& ie = _inputElements[i];
				if (ie.Format != fmt) {
					inElementsDirty = true;
					ie.Format = fmt;
				}
			}

			if (inElementsDirty) _curInLayout = _getOrCreateInputLayout();

			if (_curInLayout) {
				if (shaderParamGetter) {
					_useParameters<ProgramStage::VS>(_vsParamLayout, *shaderParamGetter);
					_useParameters<ProgramStage::PS>(_psParamLayout, *shaderParamGetter);
				}

				context->IASetInputLayout(_curInLayout);

				return true;
			}
		}
		return false;
	}

	void Program::useEnd() {
		for (auto& i : _usingSameConstBuffers) i = nullptr;
	}

	ConstantBuffer* Program::_getConstantBuffer(const MyConstantBufferLayout& cbLayout, const IShaderParameterGetter& paramGetter) {
		if (cbLayout.sameId) {
			if (auto cb = _usingSameConstBuffers[cbLayout.sameId - 1]; cb) return cb;
		}

		ShaderParameterUsageStatistics statistics;
		cbLayout.collectUsingInfo(paramGetter, statistics, (std::vector<const ShaderParameter*>&)_tempParams, _tempVars);

		ConstantBuffer* cb = nullptr;
		if (decltype(statistics.unknownCount) numVars = _tempVars.size(); statistics.unknownCount < numVars) {
			auto g = _graphics.get<Graphics>();

			if (statistics.exclusiveCount && !statistics.shareCount) {
				if (cb = (ConstantBuffer*)g->getConstantBufferManager().getExclusiveConstantBuffer(_tempParams, cbLayout); cb) {
					if (g->getInternalFeatures().MapNoOverwriteOnDynamicConstantBuffer) {
						auto isMaping = false;
						for (decltype(numVars) i = 0; i < numVars; ++i) {
							if (auto param = _tempParams[i]; param && param->getUpdateId() != cb->recordUpdateIds[i]) {
								if (!isMaping) {
									if (cb->map(Usage::MAP_WRITE) == Usage::NONE) break;
									isMaping = true;
								}
								cb->recordUpdateIds[i] = param->getUpdateId();
								ConstantBufferManager::updateConstantBuffer(cb, *param, *_tempVars[i]);
							}
						}

						if (isMaping) cb->unmap();
					} else {
						auto needUpdate = false;
						for (decltype(numVars) i = 0; i < numVars; ++i) {
							if (auto param = _tempParams[i]; param && param->getUpdateId() != cb->recordUpdateIds[i]) {
								needUpdate = true;
								cb->recordUpdateIds[i] = param->getUpdateId();
							}
						}
						if (needUpdate) _constantBufferUpdateAll(cb, cbLayout.variables);
					}
				}
			} else {
				if (cb = (ConstantBuffer*)g->getConstantBufferManager().popShareConstantBuffer(cbLayout.size); cb) _constantBufferUpdateAll(cb, cbLayout.variables);
			}
		}

		_tempParams.clear();
		_tempVars.clear();
		if (cb && cbLayout.sameId) _usingSameConstBuffers[cbLayout.sameId - 1] = cb;

		return cb;
	}

	void Program::_constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars) {
		if (cb->map(Usage::MAP_WRITE) != Usage::NONE) {
			for (size_t i = 0, n = _tempVars.size(); i < n; ++i) {
				if (auto param = _tempParams[i]; param) ConstantBufferManager::updateConstantBuffer(cb, *param, *_tempVars[i]);
			}

			cb->unmap();
		}
	}

	void Program::destroy() {
		_curInLayout = nullptr;
		_inVerBufSlots.clear();
		_inLayouts.clear();
		_info.clear();

		if (_numInElements > 0) {
			for (decltype(_numInElements) i = 0; i < _numInElements; ++i) delete[] _inputElements[i].SemanticName;
			delete[] _inputElements;
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

	ID3DBlob* Program::_compileShader(const ProgramSource& source, ProgramStage stage, const std::string_view& target, const D3D_SHADER_MACRO* d3dDefines, const ProgramDefine* defines, size_t numDefines, const ProgramIncludeHandler& includeHandler, const ProgramTranspileHandler& transpileHandler) {
		if (!source.isValid()) {
			_graphics.get<Graphics>()->error("d3d compile failed, source is invalid");
			return nullptr;
		}

		if (source.language != ProgramLanguage::HLSL) {
			if (transpileHandler) {
				ProgramTranspileInfo pti;
				pti.source = &source;
				pti.targetLanguage = ProgramLanguage::HLSL;
				pti.targetVersion = target;
				pti.defines = defines;
				pti.numDefines = numDefines;
				pti.includeHandler = includeHandler;

				auto newSource = transpileHandler(pti);
				if (!newSource.isValid()) {
					_graphics.get<Graphics>()->error("to hlsl transpile failed");
					return nullptr;
				}

				if (newSource.language != ProgramLanguage::HLSL) {
					_graphics.get<Graphics>()->error("transpiled language isnot hlsl");
					return nullptr;
				}

				return _compileShader(newSource, target, d3dDefines, includeHandler);
			} else {
				return 0;
			}
		}

		return _compileShader(source, target, d3dDefines, includeHandler);
	}

	ID3DBlob* Program::_compileShader(const ProgramSource& source, const std::string_view& target, const D3D_SHADER_MACRO* d3dDefines, const ProgramIncludeHandler& includeHandler) {
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		if (_graphics.get<Graphics>()->isDebug()) {
			shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		} else {
			shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
		}

		ID3DBlob* buffer = nullptr, *errorBuffer = nullptr;
		MyIncludeHandler include(includeHandler);
		HRESULT hr = D3DCompile(source.data.getSource(), source.data.getLength(), nullptr, d3dDefines, &include,
			source.getEntryPoint().data(), target.data(), shaderFlags, 0, &buffer, &errorBuffer);

		if (FAILED(hr)) {
			if (errorBuffer) {
				_graphics.get<Graphics>()->error(std::string("D3D shader compile error : ") + (char*)errorBuffer->GetBufferPointer());
				errorBuffer->Release();
			} else {
				_graphics.get<Graphics>()->error("D3D shader compile error");
			}

			return nullptr;
		} else if (errorBuffer) {
			errorBuffer->Release();
		}

		return buffer;
	}

	ID3D11InputLayout* Program::_getOrCreateInputLayout() {
		for (size_t i = 0, n = _inLayouts.size(); i < n; ++i) {
			if (auto& il = _inLayouts[i]; il.equal(_inputElements, _numInElements)) return il.layout;
		}

		auto& il = _inLayouts.emplace_back(_numInElements);
		for (decltype(_numInElements) i = 0; i < _numInElements; ++i) il.formats[i] = _inputElements[i].Format;
		auto hr = _graphics.get<Graphics>()->getDevice()->CreateInputLayout(_inputElements, _numInElements, _vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), &il.layout);
		return il.layout;
	}

	void Program::_parseInputLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, const ProgramInputHandler& handler) {
		if (_numInElements = desc.InputParameters; _numInElements > 0) {
			uint32_t offset = 0;
			_inputElements = new D3D11_INPUT_ELEMENT_DESC[desc.InputParameters];
			D3D11_SIGNATURE_PARAMETER_DESC pDesc;
			char nameBuf[256];
			for (decltype(_numInElements) i = 0; i < _numInElements; ++i) {
				ref.GetInputParameterDesc(i, &pDesc);

				auto& ieDesc = _inputElements[i];
				auto len = strlen(pDesc.SemanticName);
				auto name = new char[len + 1];
				ieDesc.SemanticName = name;
				name[len] = 0;
				memcpy(name, pDesc.SemanticName, len);

				memcpy(nameBuf, pDesc.SemanticName, len);
				auto nameSize = StringUtility::toString(nameBuf + len, sizeof(nameBuf) - len - 1, pDesc.SemanticIndex);
				if (nameSize == std::string::npos) nameSize = 0;
				nameSize += len;
				nameBuf[nameSize] = 0;
				ProgramInputDescriptor inputDesc;
				if (handler) {
					ProgramInputInfo pii;
					pii.name = std::string_view(nameBuf, nameSize);
					inputDesc = handler(pii);
				}

				//ieDesc.SemanticName = spDesc.SemanticName;
				ieDesc.SemanticIndex = pDesc.SemanticIndex;
				ieDesc.InputSlot = i;
				ieDesc.AlignedByteOffset = 0;
				if (inputDesc.instanced) {
					ieDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
					ieDesc.InstanceDataStepRate = 1;
				} else {
					ieDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					ieDesc.InstanceDataStepRate = 0;
				}

				_inVerBufSlots.emplace_back(i);

				auto& info = _info.vertices.emplace_back();
				info.name = std::move(std::string(nameBuf, nameSize));
				info.instanced = inputDesc.instanced;

				if (pDesc.Mask == 1) {
					info.format.dimension = 1;
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32_UINT;
						info.format.type = VertexType::UI32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32_SINT;
						info.format.type = VertexType::I32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32_FLOAT;
						info.format.type = VertexType::F32;
					}
					offset += 4;
				} else if (pDesc.Mask <= 3) {
					info.format.dimension = 2;
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_UINT;
						info.format.type = VertexType::UI32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_SINT;
						info.format.type = VertexType::I32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
						info.format.type = VertexType::F32;
					}
					offset += 8;
				} else if (pDesc.Mask <= 7) {
					info.format.dimension = 3;
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
						info.format.type = VertexType::UI32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
						info.format.type = VertexType::I32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
						info.format.type = VertexType::F32;
					}
					offset += 12;
				} else if (pDesc.Mask <= 15) {
					info.format.dimension = 4;
					if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
						info.format.type = VertexType::UI32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
						info.format.type = VertexType::I32;
					} else if (pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
						info.format.type = VertexType::F32;
					}
					offset += 16;
				}
			}
		}
	}

	void Program::_parseParameterLayout(const D3D11_SHADER_DESC& desc, ID3D11ShaderReflection& ref, ParameterLayout& dst) {
		using namespace srk::literals;

		D3D11_SHADER_INPUT_BIND_DESC ibDesc;
		for (UINT i = 0; i < desc.BoundResources; ++i) {
			ref.GetResourceBindingDesc(i, &ibDesc);

			switch (ibDesc.Type) {
			case D3D_SIT_CBUFFER:
			{
				auto& buffer = dst.constantBuffers.emplace_back();
				buffer.name = ibDesc.Name;
				buffer.bindPoint = ibDesc.BindPoint;

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
		for (UINT i = 0; i < desc.ConstantBuffers; ++i) {
			auto cb = ref.GetConstantBufferByIndex(i);
			cb->GetDesc(&bDesc);

			MyConstantBufferLayout* buffer = nullptr;
			int16_t idx = -1;
			for (int16_t j = 0, n = dst.constantBuffers.size(); j < n;  ++j) {
				if (StringUtility::equal(dst.constantBuffers[j].name.data(), bDesc.Name)) {
					idx = j;
					buffer = &dst.constantBuffers[j];
					buffer->size = bDesc.Size;
					break;
				}
			}

			if (buffer) {
				for (UINT j = 0; j < bDesc.Variables; ++j) {
					auto var = cb->GetVariableByIndex(j);
					var->GetDesc(&vDesc);

					if (vDesc.uFlags & D3D_SVF_USED) {
						auto& v = buffer->variables.emplace_back();
						v.name = vDesc.Name;
						v.offset = vDesc.StartOffset;
						v.size = vDesc.Size;
						v.stride = (1_ui32 << 31) | 16;

						_parseConstantVar(v, var->GetType());
					}
				}

				buffer->calcFeatureValue();

				_graphics.get<Graphics>()->getConstantBufferManager().registerConstantLayout(*buffer);
			}
		}
	}

	void Program::_parseConstantVar(ConstantBufferLayout::Variables& var, ID3D11ShaderReflectionType* type) {
		using namespace srk::literals;

		D3D11_SHADER_TYPE_DESC desc;
		type->GetDesc(&desc);
		var.offset += desc.Offset;
		if (desc.Class == D3D_SVC_STRUCT) {
			var.members.resize(desc.Members);
			for (uint32_t i = 0; i < desc.Members; ++i) {
				auto& memberVar = var.members[i];
				memberVar.name = type->GetMemberTypeName(i);
				memberVar.offset = var.offset;
				memberVar.size = 0;
				memberVar.stride = (1_ui32 << 31) | 16;
				
				_parseConstantVar(memberVar, type->GetMemberTypeByIndex(i));
			}
		} else if (!var.size) {
			switch (desc.Type) {
			case D3D_SVT_INT:
			case D3D_SVT_FLOAT:
				var.size = desc.Columns << 2;
				break;
			default:
				break;
			}

			if (desc.Elements > 1 && var.size) {
				auto size = var.size & 16;
				if (size) size = 16 - size;
				var.size += (var.size + size) * (desc.Elements - 1);
			}
		}
	}

	void Program::_calcConstantLayoutSameBuffers(std::vector<std::vector<MyConstantBufferLayout>*>& constBufferLayouts) {
		uint32_t sameId = 0;
		auto n = constBufferLayouts.size();
		for (size_t i = 0; i < n; ++i) {
			auto buffers0 = constBufferLayouts[i];
			for (size_t j = 0; j < n; ++j) {
				if (i == j) continue;

				auto buffers1 = constBufferLayouts[j];

				for (auto& buffer0 : *buffers0) {
					for (auto& buffer1 : *buffers1) {
						if (buffer0.featureValue == buffer1.featureValue) {
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