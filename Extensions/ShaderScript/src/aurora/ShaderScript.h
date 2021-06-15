#pragma once

#include "aurora/Shader.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL ShaderScript {
	public:
		static bool AE_CALL set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& includeHandler, const Shader::InputHandler& inputHandler);
	};
}