#pragma once

#include "aurora/modules/IModule.h"
#include "aurora/ProgramSource.h"
#include <functional>

namespace aurora {
	class ShaderDefine;
}

namespace aurora::modules::graphics {
	class AE_FW_DLL IShaderTranspiler : public IModule {
	public:
		inline constexpr static char COMBINED_TEXTURE_SAMPLER[25] = { "_CombinedTextureSampler_" };

		using IncludeHandler = std::function<ByteArray(const std::string_view&)>;

		virtual ModuleType AE_CALL getType() const override;
		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string_view& targetVersion, const ShaderDefine* defines, size_t numDefines, const IncludeHandler& callback) = 0;
	};
}