#include "VertexBuffer.h"
#include "modules/graphics/GraphicsModule.h"

namespace aurora::modules::graphics {
	VertexBuffer::VertexBuffer(GraphicsModule& graphics) : GObject(graphics) {
	}

	VertexBuffer::~VertexBuffer() {
	}
}