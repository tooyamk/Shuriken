#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) override;
		virtual void AE_CALL use() override;

	protected:
		GLuint _handle;

		void AE_CALL _release();
		GLuint AE_CALL _compileShader(const ProgramSource& source, GLenum type);
	};
}