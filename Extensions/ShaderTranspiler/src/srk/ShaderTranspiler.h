#pragma once

#include "srk/modules/graphics/GraphicsModule.h"

namespace srk::extensions {
	namespace shader_transpiler {
		class Impl;
	}

	class SRK_EXTENSION_DLL ShaderTranspiler : public Ref {
	public:
		struct SRK_EXTENSION_DLL Options {
			struct {
				uint32_t descriptorSet0BindingOffset = 0;
			} spirv;
		};

		virtual ~ShaderTranspiler();

		modules::graphics::ProgramSource SRK_CALL translate(const modules::graphics::ProgramSource& source, const Options& options, modules::graphics::ProgramLanguage targetLanguage, const std::string_view& targetVersion, const modules::graphics::ProgramDefine* defines, size_t numDefines, const modules::graphics::ProgramIncludeHandler& handler);
		static IntrusivePtr<ShaderTranspiler> SRK_CALL create();
		static IntrusivePtr<ShaderTranspiler> SRK_CALL create(const std::string_view& dxcompiler);

	private:
		ShaderTranspiler(shader_transpiler::Impl* impl);

		shader_transpiler::Impl* _impl;
	};
}