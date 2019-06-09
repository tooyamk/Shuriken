#include "VertexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
		_vertSize(VertexSize::UNKNOWN),
		_vertType(VertexType::UNKNOWN),
		_validVertexFormat(false),
		_vertexSize(0),
		_vertexType(0),
		_baseBuffer(GL_ARRAY_BUFFER) {
	}

	VertexBuffer::~VertexBuffer() {
	}

	const void* VertexBuffer::getNativeBuffer() const {
		return this;
	}

	bool VertexBuffer::create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data);
	}

	ui32 VertexBuffer::getSize() const {
		return _baseBuffer.size;
	}

	Usage VertexBuffer::getUsage() const {
		return _baseBuffer.resUsage;
	}

	Usage VertexBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage);
	}

	void VertexBuffer::unmap() {
		_baseBuffer.unmap();
	}

	ui32 VertexBuffer::read(ui32 offset, void* dst, ui32 dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	ui32 VertexBuffer::write(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.write(offset, data, length);
	}

	ui32 VertexBuffer::update(ui32 offset, const void* data, ui32 length) {
		return _baseBuffer.update(offset, data, length);
	}

	//void VertexBuffer::flush() {
	//	_baseBuffer.flush();
	//}

	bool VertexBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}

	void VertexBuffer::getFormat(VertexSize* size, VertexType* type) const {
		if (size) *size = _vertSize;
		if (type) *type = _vertType;
	}

	void VertexBuffer::setFormat(VertexSize size, VertexType type) {
		if (_vertSize != size || _vertType != type) {
			_vertSize = size;
			_vertType = type;

			switch (size) {
			case VertexSize::ONE:
				_vertexSize = 1;
				break;
			case VertexSize::TWO:
				_vertexSize = 2;
				break;
			case VertexSize::THREE:
				_vertexSize = 3;
				break;
			case VertexSize::FOUR:
				_vertexSize = 4;
				break;
			default:
				_vertexSize = 0;
				break;
			}

			switch (type) {
			case VertexType::I8:
				_vertexType = GL_BYTE;
				break;
			case VertexType::UI8:
				_vertexType = GL_UNSIGNED_BYTE;
				break;
			case VertexType::I16:
				_vertexType = GL_SHORT;
				break;
			case VertexType::UI16:
				_vertexType = GL_UNSIGNED_SHORT;
				break;
			case VertexType::I32:
				_vertexType = GL_INT;
				break;
			case VertexType::UI32:
				_vertexType = GL_UNSIGNED_INT;
				break;
			case VertexType::F32:
				_vertexType = GL_FLOAT;
				break;
			default:
				_vertexType = 0;
				break;
			}

			_validVertexFormat = _vertexSize && _vertexType;
		}
	}

	bool VertexBuffer::use(GLuint index) {
		if (_baseBuffer.handle && _validVertexFormat) {
			glEnableVertexAttribArray(index);
			glBindBuffer(GL_ARRAY_BUFFER, _baseBuffer.handle);
			glVertexAttribPointer(index, _vertexSize, _vertexType, GL_FALSE, 0, 0);
		}
		return false;
	}
}