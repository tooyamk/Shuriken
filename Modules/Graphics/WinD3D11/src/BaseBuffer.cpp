#include "BaseBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	BaseBuffer::BaseBuffer(UINT bufferType) : BaseResource(bufferType) {
	}

	BaseBuffer::~BaseBuffer() {
	}

	bool BaseBuffer::allocate(Graphics* graphics, ui32 size, Usage resUsage, const void* data, ui32 dataSize) {
		releaseRes(graphics);

		this->size = size;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		calcAllocateUsage(resUsage, size, dataSize, 0, cpuUsage, d3dUsage);

		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuUsage;
		desc.Usage = d3dUsage;
		desc.BindFlags = bindType;
		desc.ByteWidth = size;

		HRESULT hr;
		if (dataSize >= size) {
			D3D11_SUBRESOURCE_DATA res;
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
			hr = graphics->getDevice()->CreateBuffer(&desc, &res, (ID3D11Buffer**)&handle);
		} else {
			hr = graphics->getDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&handle);
		}
		if (FAILED(hr)) {
			releaseRes(graphics);
			return false;
		}

		return true;
	}

	i32 BaseBuffer::write(Graphics* graphics, ui32 offset, const void* data, ui32 length) {
		if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);
				memcpy((i8*)mappedRes.pData + offset, data, length);
				return length;
			}
			return 0;
		} else if ((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);
				if (length == size) {
					graphics->getContext()->UpdateSubresource(handle, 0, nullptr, data, 0, 0);
				} else {
					D3D11_BOX box;
					box.back = 1;
					box.front = 0;
					box.top = 0;
					box.bottom = 1;
					box.left = offset;
					box.right = offset + length;
					graphics->getContext()->UpdateSubresource(handle, 0, &box, data, 0, 0);
				}
				return length;
			}
			return 0;
		}
		return -1;
	}
}