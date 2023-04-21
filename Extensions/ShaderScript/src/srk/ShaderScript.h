#pragma once

#include "srk/Shader.h"

#ifdef SRK_EXT_SHD_SPT_EXPORTS
#	define SRK_EXT_SHD_SPT_DLL SRK_DLL_EXPORT
#else
#	define SRK_EXT_SHD_SPT_DLL SRK_DLL_IMPORT
#endif

namespace srk::extensions {
	class SRK_EXT_SHD_SPT_DLL ShaderScript {
	public:
		static bool SRK_CALL set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const modules::graphics::ProgramIncludeHandler& includeHandler, const modules::graphics::ProgramInputHandler& inputHandler, const modules::graphics::ProgramTranspileHandler& transpileHandler);
	};
}