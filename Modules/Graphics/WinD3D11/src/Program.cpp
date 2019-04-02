#include "Program.h"
#include "Graphics.h"
#include <vector>

namespace aurora::modules::graphics_win_d3d11 {
	Program::Program(Graphics& graphics) : IGraphicsProgram(graphics),
		_vs(nullptr),
		_ps(nullptr) {
	}

	Program::~Program() {
		_release();
	}

	bool Program::upload(const i8* vert, const i8* frag) {
		_release();

		if (!vert || !frag) return false;

		auto vertBuffer = _compileShader(vert, "main", "vs_4_0");
		if (!vertBuffer) return false;

		auto fragBuffer = _compileShader(frag, "main", "ps_4_0");
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
		vertBuffer->Release();
		if (FAILED(hr)) {
			fragBuffer->Release();
			_release();
			return false;
		}

		D3D11_SHADER_DESC sDesc;
		sr->GetDesc(&sDesc);
		std::vector<D3D11_INPUT_ELEMENT_DESC> ieDescs;
		ui32 offset = 0;
		for (ui32 i = 0; i < sDesc.InputParameters; ++i) {
			D3D11_SIGNATURE_PARAMETER_DESC spDesc;
			sr->GetInputParameterDesc(i, &spDesc);

			auto& ieDesc = ieDescs.emplace_back();
			ieDesc.SemanticName = spDesc.SemanticName;
			ieDesc.SemanticIndex = spDesc.SemanticIndex;
			ieDesc.InputSlot = 0;
			ieDesc.AlignedByteOffset = offset;
			ieDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			ieDesc.InstanceDataStepRate = 0;

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
		}
		sr->Release();

		ID3D11PixelShader* fs = nullptr;
		hr = device->CreatePixelShader(fragBuffer->GetBufferPointer(), fragBuffer->GetBufferSize(), 0, &_ps);
		fragBuffer->Release();
		if (FAILED(hr)) {
			_release();
			return false;
		}

		return true;
	}

	void Program::use() {
		if (_vs) {
			auto context = ((Graphics*)_graphics)->getContext();

			context->VSSetShader(_vs, 0, 0);
			context->PSSetShader(_ps, 0, 0);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->Draw(3, 0);
		}
	}

	void Program::_release() {
		if (_vs) {
			_vs->Release();
			_vs = nullptr;

			_ps->Release();
			_ps = nullptr;
		}
	}

	ID3DBlob* Program::_compileShader(const i8* source, const i8* entry, const i8* target) {
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef AE_DEBUG
		shaderFlags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* buffer = nullptr, *errorBuffer = nullptr;
		HRESULT hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr, entry, target, shaderFlags, 0, &buffer, &errorBuffer);

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