#include "IndexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	IndexBuffer::IndexBuffer(Graphics& graphics) : IIndexBuffer(graphics),
		_idxType(IndexType::UNKNOWN),
		_internalFormat(DXGI_FORMAT_UNKNOWN),
		_numElements(0),
		_baseBuffer(D3D11_BIND_INDEX_BUFFER) {
	}

	IndexBuffer::~IndexBuffer() {
		_baseBuffer.releaseBuffer(*_graphics.get<Graphics>());
	}

	const void* IndexBuffer::getNativeBuffer() const {
		return this;
	}

	bool IndexBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	ui32 IndexBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage IndexBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage IndexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(*_graphics.get<Graphics>(), expectMapUsage);
	}

	void IndexBuffer::unmap() {
		_baseBuffer.unmap(*_graphics.get<Graphics>());
	}

	ui32 IndexBuffer::read(ui32 offset, void* dst, ui32 dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	ui32 IndexBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write(*_graphics.get<Graphics>(), offset, data, length);
	}

	ui32 IndexBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.update(*_graphics.get<Graphics>(), offset, data, length);
	}

	//void IndexBuffer::flush() {
	//}

	bool IndexBuffer::isSyncing() const {
		return false;
	}

	IndexType IndexBuffer::getFormat() const {
		return _idxType;
	}

	void IndexBuffer::setFormat(IndexType type) {
		if (_idxType != type) {
			_idxType = type;

			switch (type) {
			case IndexType::UI8:
			{
				println("IndexBuffer.setFormat error : not supprot ui8 type");
				_internalFormat = DXGI_FORMAT_UNKNOWN;

				break;
			}
			case IndexType::UI16:
				_internalFormat = DXGI_FORMAT_R16_UINT;
				break;
			case IndexType::UI32:
				_internalFormat = DXGI_FORMAT_R32_UINT;
				break;
			default:
				_internalFormat = DXGI_FORMAT_UNKNOWN;
				break;
			}

			_calcNumElements();
		}
	}

	void IndexBuffer::_calcNumElements() {
		if (_baseBuffer.size && _internalFormat != DXGI_FORMAT_UNKNOWN) {
			switch (_internalFormat) {
			case DXGI_FORMAT_R16_UINT:
				_numElements = _baseBuffer.size >> 1;
				break;
			case DXGI_FORMAT_R32_UINT:
				_numElements = _baseBuffer.size >> 2;
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