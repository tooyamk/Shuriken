#pragma once

#include "Base.h"
#include "aurora/modules/graphics/ConstantBufferManager.h"

namespace aurora::modules::graphics::gl {
	class Graphics;
	class ConstantBuffer;

	class AE_MODULE_DLL Program : public IProgram {
	public:
		Program(Graphics& graphics);
		virtual ~Program();

		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL create(const ProgramSource& vert, const ProgramSource& frag, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler) override;
		virtual const ProgramInfo& getInfo() const override;
		virtual void AE_CALL destroy() override;

		bool AE_CALL use(const IVertexBufferGetter* vertexBufferGetter, const IShaderParameterGetter* shaderParamGetter);

	protected:
		struct InVertexBufferInfo {
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
		std::vector<GLuint> _inVertexBufferLocations;
		std::vector<UniformInfo> _uniformLayouts;
		std::vector<ConstantBufferLayout> _uniformBlockLayouts;

		ProgramInfo _info;

		std::vector<ShaderParameter*> _tempParams;
		std::vector<const ConstantBufferLayout::Variables*> _tempVars;

		//void _getTexture(const ShaderParameterFactory& paramFactory, const ConstantBufferLayout& layout, );

		void _constantBufferUpdateAll(ConstantBuffer* cb, const std::vector<ConstantBufferLayout::Variables>& vars);

		GLuint AE_CALL _compileShader(const ProgramSource& source, GLenum type, ProgramStage stage, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& handler);
	};
}