#include "Program.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win::glew {
	Program::Program(Graphics& graphics) : aurora::modules::graphics::Program(graphics),
		_handle(glCreateProgram()) {
	}

	Program::~Program() {
	}
}