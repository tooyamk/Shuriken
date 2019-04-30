#include "IndexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	IndexBuffer::IndexBuffer(Graphics& graphics) : IIndexBuffer(graphics),
		_indexType(0),
		_numElements(0),
		_baseBuffer(GL_ELEMENT_ARRAY_BUFFER) {
	}

	IndexBuffer::~IndexBuffer() {
	}

	bool IndexBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		auto rst = _baseBuffer.create(size, bufferUsage, data);
		_calcNumElements();
		return rst;
	}

	Usage IndexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void IndexBuffer::unmap() {
		_baseBuffer.unmap();
	}

	i32 IndexBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return _baseBuffer.read(offset, dst, dstLen, readLen);
	}

	i32 IndexBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write(offset, data, length);
	}

	i32 IndexBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.update(offset, data, length);
	}

	void IndexBuffer::flush() {
		_baseBuffer.flush();
	}

	void IndexBuffer::setFormat(IndexType type) {
		switch (type) {
		case IndexType::UI8:
			_indexType = GL_UNSIGNED_BYTE;
			break;
		case IndexType::UI16:
			_indexType = GL_UNSIGNED_SHORT;
			break;
		case IndexType::UI32:
			_indexType = GL_UNSIGNED_INT;
			break;
		default:
			_indexType = 0;
			break;
		}

		_calcNumElements();
	}

	void IndexBuffer::draw(ui32 count, ui32 offset) {
		if (_numElements > 0 && offset < _numElements) {
			ui32 last = _numElements - offset;
			if (count > _numElements) count = _numElements;
			if (count > last) count = last;

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _baseBuffer.handle);
			glDrawRangeElements(GL_TRIANGLES, offset, _numElements, count, _indexType, nullptr);
		}
	}

	void IndexBuffer::_calcNumElements() {
		if (_baseBuffer.size && _indexType) {
			switch (_indexType) {
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