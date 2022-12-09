#pragma once

#include "Base.h"
#include "srk/modules/graphics/ConstantBufferManager.h"

namespace srk::modules::graphics::gl {
	class Graphics;
	class ConstantBuffer;

	class SRK_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& includeHandler, const InputHandler& inputHandler) override;
		virtual const ProgramInfo& getInfo() const override;
		virtual void SRK_CALL destroy() override;

		bool SRK_CALL use(const IVertexBufferGetter* vertexBufferGetter, const IShaderParameterGetter* shaderParamGetter);

	protected:
		struct InputVertexBufferInfo {
			GLuint location;
			GLenum type;
			GLint size;
			std::string name;
		};

		
		struct UniformInfo {
			GLuint location = 0;
			GLenum type = 0;
			GLint size;
			std::vector<std::string> names;
		};


		GLuint _handle;
		std::vector<GLuint> _inputVertexBufferLocations;
		std::vector<UniformInfo> _uniformLayouts;
		std::vector<ConstantBufferLayout> _uniformBlockLayouts;

		ProgramInfo _info;

		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		//void _getTexture(const ShaderParameterFactory& paramFactory, const ConstantBufferLayout& layout, );

		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars);

		GLuint SRK_CALL _compileShader(const ProgramSource& source, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler);
	};
}