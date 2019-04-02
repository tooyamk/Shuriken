#include "IndexBuffer.h"
#include "Graphics.h"

namespace aurora::modules::graphics_win_glew {
	IndexBuffer::IndexBuffer(Graphics& graphics) : IGraphicsIndexBuffer(graphics) {
	}

	IndexBuffer::~IndexBuffer() {
	}
}