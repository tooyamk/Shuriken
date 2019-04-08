#include "VertexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	VertexBuffer::VertexBuffer(Graphics& graphics) : BaseBuffer(GL_ARRAY_BUFFER), IVertexBuffer(graphics),
		_validVertexFormat(false),
		_vertexSize(0),
		_vertexType(0) {
	}

	VertexBuffer::~VertexBuffer() {
	}

	bool VertexBuffer::stroage(ui32 size, const void* data) {
		return _stroage(size, data);
	}

	void VertexBuffer::write(ui32 offset, const void* data, ui32 length) {
		_write(offset, data, length);
	}

	void VertexBuffer::flush() {
		_flush();
	}

	void VertexBuffer::setFormat(VertexSize size, VertexType type) {
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

	bool VertexBuffer::use(GLuint index) {
		if (_handle && _validVertexFormat) {
			_waitServerSync();

			glEnableVertexAttribArray(index);
			glBindBuffer(GL_ARRAY_BUFFER, _handle);
			glVertexAttribPointer(index, _vertexSize, _vertexType, GL_FALSE, 0, 0);
		}
		return false;
	}
}