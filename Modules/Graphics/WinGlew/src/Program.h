#pragma once

#include "Base.h"

namespace aurora::modules::graphics_win_glew {
	class Graphics;

	class AE_MODULE_DLL Program : public GraphicsModule::Program {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual void AE_CALL use() override;

	protected:
		ui32 _handle;

		GLuint _compileShader(const GLchar* source, GLenum type);
	};
}