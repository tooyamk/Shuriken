#pragma once

#include "aurora/ByteArray.h"

namespace aurora::modules::graphics {
	enum class ProgramStage : uint8_t;


	enum class ProgramLanguage : uint8_t {
		UNKNOWN,
		HLSL,
		DXIL,
		SPIRV,
		GLSL,
		GSSL,
		MSL
	};


	class AE_DLL ProgramSource {
	public:
		ProgramSource();
		ProgramSource(ProgramSource&& value);

		ProgramLanguage language;
		ProgramStage stage;
		std::string version;
		std::string entryPoint;
		ByteArray data;

		ProgramSource& AE_CALL operator=(ProgramSource&& value);

		bool AE_CALL isValid() const;

		inline static std::string AE_CALL toHLSLShaderModel(const ProgramSource& source) {
			return toHLSLShaderModel(source.stage, source.version);
		}

		static std::string AE_CALL toHLSLShaderModel(ProgramStage stage, const std::string_view& version);

		inline static std::string AE_CALL getEntryPoint(const ProgramSource& source) {
			return getEntryPoint(source.entryPoint);
		}
		inline static std::string AE_CALL getEntryPoint(const std::string_view& entryPoint) {
			return entryPoint.empty() ? "main" : std::move(std::string(entryPoint));
		}
	};
}