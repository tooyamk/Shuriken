#pragma once

#include "Base.h"

namespace aurora::modules::graphics_win_glew {
	class Graphics;

	class AE_MODULE_DLL Program : public IGraphicsProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual bool AE_CALL upload(const i8* vert, const i8* frag) override;
		virtual void AE_CALL use() override;

	protected:
		GLuint _handle;

		void AE_CALL _release();
		GLuint AE_CALL _compileShader(const GLchar* source, GLenum type);
	};
}