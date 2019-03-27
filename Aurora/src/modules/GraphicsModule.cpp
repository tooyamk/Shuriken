#include "GraphicsModule.h"

namespace aurora::modules {
	GraphicsModule::~GraphicsModule() {
	}


	GraphicsModule::Object::Object(GraphicsModule& graphics) :
		_graphics(&graphics) {
		_graphics->ref();
	}

	GraphicsModule::Object::~Object() {
		_graphics->unref();
	}


	GraphicsModule::Program::Program(GraphicsModule& graphics) : Object(graphics) {
	}

	GraphicsModule::Program::~Program() {
	}


	GraphicsModule::VertexBuffer::VertexBuffer(GraphicsModule& graphics) : Object(graphics) {
	}

	GraphicsModule::VertexBuffer::~VertexBuffer() {
	}
}