#pragma once

#include "srk/modules/IModule.h"
#include "srk/ProgramSource.h"
#include <functional>

namespace srk {
	class ShaderDefine;
}

namespace srk::modules::graphics {
	class SRK_FW_DLL IShaderTranspiler : public IModule {
	public:
		inline constexpr static char COMBINED_TEXTURE_SAMPLER[25] = { "_CombinedTextureSampler_" };

		using IncludeHandler = std::function<ByteArray(const std::string_view&)>;

		virtual ModuleType SRK_CALL getType() const override;
		virtual ProgramSource SRK_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string_view& targetVersion, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& callback) = 0;
	};
}