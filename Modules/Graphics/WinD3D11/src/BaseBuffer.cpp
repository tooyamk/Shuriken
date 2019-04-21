#include "BaseBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	BaseBuffer::BaseBuffer(Graphics& graphics, UINT bufferType) : BaseResource(graphics, bufferType) {
	}

	BaseBuffer::~BaseBuffer() {
	}

	bool BaseBuffer::_allocate(ui32 size, ui32 resUsage, const void* data) {
		_delRes();

		_size = size;

		D3D11_USAGE d3dUsage;
		UINT cpuUsage;
		_calcAllocateUsage(resUsage, data, cpuUsage, d3dUsage);

		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuUsage;
		desc.Usage = d3dUsage;
		desc.BindFlags = _bindType;
		desc.ByteWidth = size;

		HRESULT hr;
		if (data) {
			D3D11_SUBRESOURCE_DATA res;
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
			hr = _grap->getDevice()->CreateBuffer(&desc, &res, (ID3D11Buffer**)&_handle);
		} else {
			hr = _grap->getDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&_handle);
		}
		if (FAILED(hr)) {
			_delRes();
			return false;
		}

		return true;
	}
}