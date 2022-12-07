#include "VertexBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	VertexBuffer::VertexBuffer(Graphics& graphics) : IVertexBuffer(graphics),
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

	bool VertexBuffer::create(size_t size, Usage bufferUsage, const void* data, size_t dataSize) {
		return _baseBuffer.create(*_graphics.get<Graphics>(), size, bufferUsage, data);
	}

	size_t VertexBuffer::getSize() const {
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

	size_t VertexBuffer::read(size_t offset, void* dst, size_t dstLen) {
		return _baseBuffer.read(offset, dst, dstLen);
	}

	size_t VertexBuffer::write(size_t offset, const void* data, size_t length) {
		return _baseBuffer.write(offset, data, length);
	}

	size_t VertexBuffer::update(size_t offset, const void* data, size_t length) {
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

	const VertexFormat& VertexBuffer::getFormat() const {
		return _format;
	}

	void VertexBuffer::setFormat(const VertexFormat& format) {
		if (_format != format) {
			_format = format;

			switch (_format.dimension) {
			case VertexDimension::ONE:
				_vertexSize = 1;
				break;
			case VertexDimension::TWO:
				_vertexSize = 2;
				break;
			case VertexDimension::THREE:
				_vertexSize = 3;
				break;
			case VertexDimension::FOUR:
				_vertexSize = 4;
				break;
			default:
				_vertexSize = 0;
				break;
			}

			switch (_format.type) {
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

	bool VertexBuffer::use(GLuint index, bool instanced) {
		if (_baseBuffer.handle && _validVertexFormat) {
			glEnableVertexAttribArray(index);
			glBindBuffer(GL_ARRAY_BUFFER, _baseBuffer.handle);
			glVertexAttribPointer(index, _vertexSize, _vertexType, GL_FALSE, 0, 0);
			glVertexAttribDivisor(index, instanced ? 1 : 0);
		}
		return false;
	}
}