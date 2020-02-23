#pragma once

#include "aurora/Shader.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL ShaderUploader {
	public:
		AE_DECLARE_CANNOT_INSTANTIATE(ShaderUploader);

		static bool AE_CALL upload(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& handler);
	};
}