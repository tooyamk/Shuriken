#include "IndexBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::d3d11 {
	IndexBuffer::IndexBuffer(Graphics& graphics) : IIndexBuffer(graphics),
		_idxType(IndexType::UNKNOWN),
		_internalFormat(DXGI_FORMAT_UNKNOWN),
		_numElements(0),
		_baseBuffer(D3D11_BIND_INDEX_BUFFER) {
	}

	IndexBuffer::~IndexBuffer() {
		destroy();
	}

	bool IndexBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* IndexBuffer::getNative() const {
		return this;
	}

	bool IndexBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data, dataSize);
	}

	size_t IndexBuffer::getSize() const {
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

	size_t IndexBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	size_t IndexBuffer::write(size_t offset, const void* data, size_t length) {
		return _baseBuffer.write(*_graphics.get<Graphics>(), offset, data, length);
	}

	size_t IndexBuffer::update(size_t offset, const void* data, size_t length) {
		return _baseBuffer.update(*_graphics.get<Graphics>(), offset, data, length);
	}

	//void IndexBuffer::flush() {
	//}

	bool IndexBuffer::isSyncing() const {
		return false;
	}

	void IndexBuffer::destroy() {
		_baseBuffer.releaseBuffer(*_graphics.get<Graphics>());
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
				_graphics.get<Graphics>()->error("D3D IndexBuffer::setFormat error : not supprot IndexType::UI8");
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