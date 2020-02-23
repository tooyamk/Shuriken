#include "ShaderUploader.h"
#include "ShaderUploaderImpl.h"

namespace aurora::extensions {
	bool ShaderUploader::upload(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& handler) {
		return shader_uploader::upload(shader, graphics, source, handler);
	}
}