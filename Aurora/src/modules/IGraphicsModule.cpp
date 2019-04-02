#include "IGraphicsModule.h"

namespace aurora::modules {
	IGraphicsModule::~IGraphicsModule() {
	}


	IGraphicsObject::IGraphicsObject(IGraphicsModule& graphics) :
		_graphics(graphics.ref<IGraphicsModule>()) {
	}

	IGraphicsObject::~IGraphicsObject() {
		Ref::setNull(_graphics);
	}


	IGraphicsIndexBuffer::IGraphicsIndexBuffer(IGraphicsModule& graphics) : IGraphicsObject(graphics) {
	}

	IGraphicsIndexBuffer::~IGraphicsIndexBuffer() {
	}


	IGraphicsProgram::IGraphicsProgram(IGraphicsModule& graphics) : IGraphicsObject(graphics) {
	}

	IGraphicsProgram::~IGraphicsProgram() {
	}


	IGraphicsVertexBuffer::IGraphicsVertexBuffer(IGraphicsModule& graphics) : IGraphicsObject(graphics) {
	}

	IGraphicsVertexBuffer::~IGraphicsVertexBuffer() {
	}
}