#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseBuffer::BaseBuffer(Graphics& graphics, UINT bufferType) :
		_grap(&graphics),
		_bufferType(bufferType),
		_handle(nullptr) {
	}

	BaseBuffer::~BaseBuffer() {
		_delBuffer();
	}

	bool BaseBuffer::_stroage(ui32 size, const void* data) {
		_delBuffer();

		_size = size;

		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		//desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = _bufferType;
		desc.ByteWidth = size;

		D3D11_SUBRESOURCE_DATA res;
		if (data) {
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
		}

		auto hr = _grap->getDevice()->CreateBuffer(&desc, data ? &res : nullptr, &_handle);
		if (FAILED(hr)) {
			_delBuffer();
			return false;
		}

		return true;
	}

	void BaseBuffer::_write(ui32 offset, const void* data, ui32 length) {
	}

	void BaseBuffer::_flush() {
	}

	void BaseBuffer::_delBuffer() {
		if (_handle) {
			_handle->Release();
			_handle = nullptr;
		}
		_size = 0;
	}

	void BaseBuffer::_waitServerSync() {
	}

	void BaseBuffer::_delSync() {
	}
}