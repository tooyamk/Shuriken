#include "ShaderScript.h"
#include "ShaderScriptImpl.h"

namespace aurora::extensions {
	bool ShaderScript::set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& handler) {
		return shader_script::set(shader, graphics, source, handler);
	}
}