#include "IndexBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	IndexBuffer::IndexBuffer(Graphics& graphics) : IIndexBuffer(graphics),
		_idxType(IndexType::UNKNOWN),
		_internalType(0),
		_numElements(0),
		_baseBuffer(GL_ELEMENT_ARRAY_BUFFER) {
	}

	IndexBuffer::~IndexBuffer() {
	}

	bool IndexBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* IndexBuffer::getNative() const {
		return this;
	}

	bool IndexBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		auto rst = _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data);
		_calcNumElements();
		return rst;
	}

	size_t IndexBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage IndexBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage IndexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void IndexBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t IndexBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	size_t IndexBuffer::write(size_t offset, const void* data, size_t length) {
		return _baseBuffer.write(offset, data, length);
	}

	size_t IndexBuffer::update(size_t offset, const void* data, size_t length) {
		return _baseBuffer.update(offset, data, length);
	}

	//void IndexBuffer::flush() {
	//	_baseBuffer.flush();
	//}

	bool IndexBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}

	void IndexBuffer::destroy() {
		_baseBuffer.releaseBuffer();
	}

	IndexType IndexBuffer::getFormat() const {
		return _idxType;
	}

	void IndexBuffer::setFormat(IndexType type) {
		if (_idxType != type) {
			_idxType = type;

			switch (type) {
			case IndexType::UI8:
				_internalType = GL_UNSIGNED_BYTE;
				break;
			case IndexType::UI16:
				_internalType = GL_UNSIGNED_SHORT;
				break;
			case IndexType::UI32:
				_internalType = GL_UNSIGNED_INT;
				break;
			default:
				_internalType = 0;
				break;
			}

			_calcNumElements();
		}
	}

	void IndexBuffer::draw(uint32_t count, uint32_t offset) const {
		if (_numElements && offset < _numElements) {
			uint32_t last = _numElements - offset;
			if (count > _numElements) count = _numElements;
			if (count > last) count = last;

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _baseBuffer.handle);
			glDrawRangeElements(GL_TRIANGLES, offset, _numElements, count, _internalType, nullptr);
		}
	}

	void IndexBuffer::drawInstanced(uint32_t instancedCount, uint32_t count, uint32_t offset) const {
		if (instancedCount && _numElements && offset < _numElements) {
			uint32_t last = _numElements - offset;
			if (count > _numElements) count = _numElements;
			if (count > last) count = last;

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _baseBuffer.handle);

			auto addr = (uintptr_t)(offset * Graphics::getGLTypeSize(_internalType));
			glDrawElementsInstanced(GL_TRIANGLES, count, _internalType, (const void*)addr, instancedCount);
		}
	}

	void IndexBuffer::_calcNumElements() {
		if (_baseBuffer.size && _internalType) {
			switch (_internalType) {
			case GL_UNSIGNED_BYTE:
				_numElements = _baseBuffer.size;
				break;
			case GL_UNSIGNED_SHORT:
				_numElements = _baseBuffer.size >> 1;
				break;
			case GL_UNSIGNED_INT:
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