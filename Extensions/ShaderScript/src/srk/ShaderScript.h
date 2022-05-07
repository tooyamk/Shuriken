#pragma once

#include "srk/Shader.h"

namespace srk::extensions {
	class SRK_EXTENSION_DLL ShaderScript {
	public:
		static bool SRK_CALL set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& includeHandler, const Shader::InputHandler& inputHandler);
	};
}