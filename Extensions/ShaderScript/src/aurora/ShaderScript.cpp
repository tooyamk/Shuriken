#include "ShaderScript.h"
#include "ShaderScriptImpl.h"

namespace aurora::extensions {
	bool ShaderScript::upload(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& handler) {
		return shader_script::upload(shader, graphics, source, handler);
	}
}