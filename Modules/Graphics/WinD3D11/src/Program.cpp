#include "Program.h"
#include "Graphics.h"
#include "base/String.h"
#include <vector>

namespace aurora::modules::graphics::win_d3d11 {
	Program::Program(Graphics& graphics) : IProgram(graphics),
		_vs(nullptr),
		_ps(nullptr),
		_inputLayout(nullptr) {
	}

	Program::~Program() {
		_release();
	}

	bool Program::upload(const ProgramSource& vert, const ProgramSource& frag) {
		_release();

		auto vertBuffer = _compileShader(vert, "vs_5_0");
		if (!vertBuffer) return false;

		auto fragBuffer = _compileShader(frag, "ps_5_0");
		if (!fragBuffer) {
			vertBuffer->Release();
			return false;
		}

		auto device = ((Graphics*)_graphics)->getDevice();

		HRESULT hr = device->CreateVertexShader(vertBuffer->GetBufferPointer(), vertBuffer->GetBufferSize(), 0, &_vs);
		if (FAILED(hr)) {
			vertBuffer->Release();
			fragBuffer->Release();
			_release();
			return false;
		}

		ID3D11ShaderReflection* sr = nullptr;
		hr = D3DReflect(vertBuffer->GetBufferPointer(), vertBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&sr);
		if (FAILED(hr)) {
			vertBuffer->Release();
			fragBuffer->Release();
			_release();
			return false;
		}

		D3D11_SHADER_DESC sDesc;
		sr->GetDesc(&sDesc);
		ui32 offset = 0;
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
		for (ui32 i = 0; i < sDesc.InputParameters; ++i) {
			D3D11_SIGNATURE_PARAMETER_DESC spDesc;
			sr->GetInputParameterDesc(i, &spDesc);

			auto& ieDesc = inputElements.emplace_back();
			ieDesc.SemanticName = spDesc.SemanticName;
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
				info.stride = 4;
			} else if (spDesc.Mask <= 3) {
				if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32_UINT;
				} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32_SINT;
				} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
				}
				offset += 8;
				info.stride = 8;
			} else if (spDesc.Mask <= 7) {
				if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				}
				offset += 12;
				info.stride = 12;
			} else if (spDesc.Mask <= 15) {
				if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				} else if (spDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
					ieDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				}
				offset += 16;
				info.stride = 16;
			}
		}
		sr->Release();

		hr = device->CreateInputLayout(&inputElements[0], inputElements.size(), vertBuffer->GetBufferPointer(), vertBuffer->GetBufferSize(), &_inputLayout);
		if (FAILED(hr)) {
			vertBuffer->Release();
			fragBuffer->Release();
			_release();
			return false;
		}

		ID3D11PixelShader* fs = nullptr;
		hr = device->CreatePixelShader(fragBuffer->GetBufferPointer(), fragBuffer->GetBufferSize(), 0, &_ps);
		if (FAILED(hr)) {
			vertBuffer->Release();
			fragBuffer->Release();
			_release();
			return false;
		}

		vertBuffer->Release();
		fragBuffer->Release();

		return true;
	}

	void Program::use() {
		if (_vs) {
			auto context = ((Graphics*)_graphics)->getContext();

			context->VSSetShader(_vs, 0, 0);
			context->PSSetShader(_ps, 0, 0);

			context->IASetInputLayout(_inputLayout);
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->Draw(3, 0);
		}
	}

	void Program::_release() {
		_inVerBufInfos.clear();

		if (_inputLayout) {
			_inputLayout->Release();
			_inputLayout = nullptr;
		}

		if (_vs) {
			_vs->Release();
			_vs = nullptr;
		}

		if (_ps) {
			_ps->Release();
			_ps = nullptr;
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
}