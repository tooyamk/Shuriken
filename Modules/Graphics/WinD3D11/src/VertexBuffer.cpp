#include "VertexBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics_win_d3d11 {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IGraphicsVertexBuffer(graphics),
		_handle(nullptr) {
	}

	VertexBuffer::~VertexBuffer() {
		_delBuffer();
	}

	bool VertexBuffer::stroage(ui32 size, const void* data) {
		_delBuffer();

		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		//desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = size;

		D3D11_SUBRESOURCE_DATA res;
		if (data) {
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
		}

		HRESULT hr = ((Graphics*)_graphics)->getDevice()->CreateBuffer(&desc, data ? &res : nullptr, &_handle);
		if (FAILED(hr)) {
			_delBuffer();
			return false;
		}

		return true;
	}

	void VertexBuffer::write(ui32 offset, const void* data, ui32 length) {
	}

	void VertexBuffer::flush() {
	}

	void VertexBuffer::use() {
		if (_handle) {
			auto context = ((Graphics*)_graphics)->getContext();

			UINT stride = 8;
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, &_handle, &stride, &offset);
		}
	}

	void VertexBuffer::_delBuffer() {
		if (_handle) {
			_handle->Release();
			_handle = nullptr;
		}
	}

	void VertexBuffer::_waitServerSync() {
	}

	void VertexBuffer::_delSync() {
	}
}