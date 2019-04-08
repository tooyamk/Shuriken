#include "Program.h"
#include "Graphics.h"
#include "base/String.h"
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


	Program::Program(Graphics& graphics) : IProgram(graphics),
		_inElementsDirty(false),
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

		_vertBlob = _compileShader(vert, "vs_5_0");
		if (!_vertBlob) return false;

		auto fragBuffer = _compileShader(frag, "ps_5_0");
		if (!fragBuffer) {
			_release();
			return false;
		}

		auto device = ((Graphics*)_graphics)->getDevice();

		HRESULT hr = device->CreateVertexShader(_vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), 0, &_vs);
		if (FAILED(hr)) {
			fragBuffer->Release();
			_release();
			return false;
		}

		ID3D11ShaderReflection* sr = nullptr;
		hr = D3DReflect(_vertBlob->GetBufferPointer(), _vertBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&sr);
		if (FAILED(hr)) {
			fragBuffer->Release();
			_release();
			return false;
		}

		D3D11_SHADER_DESC sDesc;
		sr->GetDesc(&sDesc);
		_numInElements = sDesc.InputParameters;
		if (_numInElements > 0) {
			ui32 offset = 0;
			_inElements = new D3D11_INPUT_ELEMENT_DESC[sDesc.InputParameters];
			for (ui32 i = 0; i < _numInElements; ++i) {
				D3D11_SIGNATURE_PARAMETER_DESC spDesc;
				sr->GetInputParameterDesc(i, &spDesc);

				auto& ieDesc = _inElements[i];
				ui32 len = strlen(spDesc.SemanticName);
				i8* name = new i8[len + 1];
				ieDesc.SemanticName = name;
				name[len] = 0;
				memcpy(name, spDesc.SemanticName, len);
				//ieDesc.SemanticName = spDesc.SemanticName;
				ieDesc.SemanticIndex = spDesc.SemanticIndex;
				ieDesc.InputSlot = i;
				ieDesc.AlignedByteOffset = 0;
				ieDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				ieDesc.InstanceDataStepRate = 0;

				auto& info = _inVerBufInfos.emplace_back();
				info.name = spDesc.SemanticName + String::toString(spDesc.SemanticIndex);
				info.slot = i;

				if (spDesc.Mask == 1) {
					if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32_UINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32_SINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32_FLOAT;
					}
					offset += 4;
				} else if (spDesc.Mask <= 3) {
					if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_UINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_SINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
					}
					offset += 8;
				} else if (spDesc.Mask <= 7) {
					if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
					}
					offset += 12;
				} else if (spDesc.Mask <= 15) {
					if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
					} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
						ieDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					}
					offset += 16;
				}

				if (i == 0) {
					ieDesc.Format = DXGI_FORMAT_R8G8_UINT;
				}
			}
		}
		sr->Release();

		_curInLayout = _getOrCreateInputLayout();
		_inElementsDirty = false;
		
		if (FAILED(hr)) {
			fragBuffer->Release();
			_release();
			return false;
		}

		ID3D11PixelShader* fs = nullptr;
		hr = device->CreatePixelShader(fragBuffer->GetBufferPointer(), fragBuffer->GetBufferSize(), 0, &_ps);
		if (FAILED(hr)) {
			fragBuffer->Release();
			_release();
			return false;
		}

		fragBuffer->Release();

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

	void Program::useVertexBuffers(const VertexBufferFactory& factory) {
		for (ui32 i = 0, n = _inVerBufInfos.size(); i < n; ++i) {
			auto& info = _inVerBufInfos[i];
			auto vb = (VertexBuffer*)factory.get(info.name);
			if (vb) {
				auto& ie = _inElements[i];
				DXGI_FORMAT fmt;
				if (vb->use(info.slot, fmt)) {
					if (ie.Format != fmt) {
						_inElementsDirty = true;
						ie.Format = fmt;
					}
				}
			}
		}
	}

	void Program::draw(const IIndexBuffer& indexBuffer, ui32 count, ui32 offset) {
		if (_vs) {
			auto g = (Graphics*)_graphics;
			auto context = g->getContext();

			if (_inElementsDirty) {
				_inElementsDirty = false;

				_curInLayout = _getOrCreateInputLayout();
			}

			if (_curInLayout) {
				context->IASetInputLayout(_curInLayout);
				((IndexBuffer&)indexBuffer).draw(count, offset);
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

		_inElementsDirty = false;

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
	}

	ID3DBlob* Program::_compileShader(const ProgramSource& source, const i8* target) {
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef AE_DEBUG1
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
}