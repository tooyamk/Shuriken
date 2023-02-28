#pragma once

#include "srk/Shader.h"

namespace srk::extensions {
	class SRK_EXTENSION_DLL ShaderScript {
	public:
		static bool SRK_CALL set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const modules::graphics::ProgramIncludeHandler& includeHandler, const modules::graphics::ProgramInputHandler& inputHandler, const modules::graphics::ProgramTranspileHandler& transpileHandler);
	};
}