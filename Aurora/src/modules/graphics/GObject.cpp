#include "GObject.h"
#include "modules/graphics/GraphicsModule.h"

namespace aurora::modules::graphics {
	GObject::GObject(GraphicsModule& graphics) :
		_graphics(&graphics) {
		_graphics->ref();
	}

	GObject::~GObject() {
		_graphics->unref();
	}
}