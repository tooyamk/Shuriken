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
		calcAllocateUsage(resUsage, size, dataSize, cpuUsage, d3dUsage);

		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuUsage;
		desc.Usage = d3dUsage;
		desc.BindFlags = _bindType;
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
}