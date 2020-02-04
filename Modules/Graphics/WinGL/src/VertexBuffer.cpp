#include "VertexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
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

	bool VertexBuffer::isCreated() const {
		return _baseBuffer.handle;
	}

	const void* VertexBuffer::getNative() const {
		return this;
	}

	bool VertexBuffer::create(uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data);
	}

	uint32_t VertexBuffer::getSize() const {
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

	uint32_t VertexBuffer::read(uint32_t offset, void* dst, uint32_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	uint32_t VertexBuffer::write(uint32_t offset, const void* data, uint32_t length) {
		return _baseBuffer.write(offset, data, length);
	}

	uint32_t VertexBuffer::update(uint32_t offset, const void* data, uint32_t length) {
		return _baseBuffer.update(offset, data, length);
	}

	//void VertexBuffer::flush() {
	//	_baseBuffer.flush();
	//}

	bool VertexBuffer::isSyncing() const {
		return _baseBuffer.isSyncing();
	}

	void VertexBuffer::destroy() {
		_baseBuffer.releaseBuffer();
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