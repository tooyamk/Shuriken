#include "IndexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	IndexBuffer::IndexBuffer(Graphics& graphics) : BaseBuffer(graphics, D3D11_BIND_INDEX_BUFFER), IIndexBuffer(graphics),
		_indexType(DXGI_FORMAT_UNKNOWN) {
	}

	IndexBuffer::~IndexBuffer() {
	}

	bool IndexBuffer::allocate(ui32 size, ui32 bufferUsage, const void* data) {
		return _allocate(size, bufferUsage,  data);
	}

	ui32 IndexBuffer::map(ui32 mapUsage) {
		return _map(mapUsage);
	}

	void IndexBuffer::unmap() {
		_unmap();
	}

	i32 IndexBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _read(offset, dst, dstLen, readLen);
	}

	i32 IndexBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _write(offset, data, length);
	}

	void IndexBuffer::flush() {
		_flush();
	}

	void IndexBuffer::setFormat(IndexType type) {
		switch (type) {
		case IndexType::UI8:
		{
			println("IndexBuffer.setFormat error : not supprot ui8 type");
			_indexType = DXGI_FORMAT_UNKNOWN;

			break;
		}
		case IndexType::UI16:
			_indexType = DXGI_FORMAT_R16_UINT;
			break;
		case IndexType::UI32:
			_indexType = DXGI_FORMAT_R32_UINT;
			break;
		default:
			_indexType = DXGI_FORMAT_UNKNOWN;
			break;
		}

		_calcNumElements();
	}

	void IndexBuffer::draw(ui32 count, ui32 offset) {
		if (_numElements > 0 && offset < _numElements) {
			ui32 last = _numElements - offset;
			if (count > _numElements) count = _numElements;
			if (count > last) count = last;

			auto context = _grap->getContext();
			context->IASetIndexBuffer((ID3D11Buffer*)_handle, _indexType, 0);
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->DrawIndexed(count, offset, 0);
		}
	}

	void IndexBuffer::_calcNumElements() {
		if (_size && _indexType != DXGI_FORMAT_UNKNOWN) {
			switch (_indexType) {
			case DXGI_FORMAT_R16_UINT:
				_numElements = _size >> 1;
				break;
			case DXGI_FORMAT_R32_UINT:
				_numElements = _size >> 2;
				break;
			default:
				_numElements = 0;
				break;
			}
		} else {
			_numElements = 0;
		}
	}
}