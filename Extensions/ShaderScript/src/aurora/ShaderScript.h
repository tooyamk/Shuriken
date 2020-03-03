#pragma once

#include "aurora/Shader.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL ShaderScript {
	public:
		AE_DECLARE_CANNOT_INSTANTIATE(ShaderScript);

		static bool AE_CALL upload(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& handler);
	};
}