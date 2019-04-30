#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual bool AE_CALL upload(const ProgramSource& vert, const ProgramSource& frag) override;
		virtual bool AE_CALL use() override;
		virtual void AE_CALL draw(const VertexBufferFactory* vertexFactory, const ShaderParameterFactory* paramFactory,
			const IIndexBuffer* indexBuffer, ui32 count = 0xFFFFFFFFui32, ui32 offset = 0) override;

	protected:
		struct InVertexBufferInfo {
			GLuint index;
			std::string name;
		};


		GLuint _handle;
		std::vector<InVertexBufferInfo> _inVerBufInfos;

		void AE_CALL _release();
		GLuint AE_CALL _compileShader(const ProgramSource& source, GLenum type);
	};
}