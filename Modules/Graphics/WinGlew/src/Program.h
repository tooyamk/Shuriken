#pragma once

#include "Base.h"
#include "modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;
	class ConstantBuffer;

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
			GLuint location;
			GLenum type;
			GLint size;
			std::string name;
		};

		
		struct UniformInfo {
			GLuint location;
			GLenum type;
			GLint size;
			std::vector<std::string> names;
		};


		GLuint _handle;
		std::vector<InVertexBufferInfo> _inVertexBufferLayouts;
		std::vector<UniformInfo> _uniformLayouts;
		std::vector<ConstantBufferLayout> _uniformBlockLayouts;

		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		//void _getTexture(const ShaderParameterFactory& paramFactory, const ConstantBufferLayout& layout, );

		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars);

		void AE_CALL _release();
		GLuint AE_CALL _compileShader(const ProgramSource& source, GLenum type);
	};
}