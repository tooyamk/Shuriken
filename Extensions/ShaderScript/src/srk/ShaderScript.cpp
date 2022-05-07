#include "ShaderScript.h"
#include "ShaderScriptImpl.h"

namespace srk::extensions {
	bool ShaderScript::set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& includeHandler, const Shader::InputHandler& inputHandler) {
		return shader_script::set(shader, graphics, source, includeHandler, inputHandler);
	}
}