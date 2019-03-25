#pragma once

#include "modules/graphics/Program.h"
#include "GL/glew.h"

namespace aurora::modules::graphics::win::glew {
	class Graphics;

	class AE_MODULE_DLL Program : public aurora::modules::graphics::Program {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual void AE_CALL use() override;

	protected:
		ui32 _handle;

		GLuint _compileShader(const GLchar* source, GLenum type);
	};
}