#include "Program.h"
#include "modules/graphics/GraphicsModule.h"

namespace aurora::modules::graphics {
	Program::Program(GraphicsModule& graphics) : GObject(graphics) {
	}

	Program::~Program() {
	}
}