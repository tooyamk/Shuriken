#include "ShaderScript.h"
#include "ShaderScriptImpl.h"

namespace srk::extensions {
	bool ShaderScript::set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const modules::graphics::ProgramIncludeHandler& includeHandler, const modules::graphics::ProgramInputHandler& inputHandler, const modules::graphics::ProgramTranspileHandler& transpileHandler) {
		return shader_script::set(shader, graphics, source, includeHandler, inputHandler, transpileHandler);
	}
}