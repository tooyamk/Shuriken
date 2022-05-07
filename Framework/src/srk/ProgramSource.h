#pragma once

#include "srk/ByteArray.h"
#include "srk/Intrusive.h"

namespace srk {
	enum class ProgramStage : uint8_t {
		UNKNOWN,
		CS,//ComputeShader
		DS, //DomainShader
		GS,//GeomtryShader
		HS,//HullShader
		PS,//PixelShader
		VS,//VertexShader
	};


	enum class ProgramLanguage : uint8_t {
		UNKNOWN,
		HLSL,
		DXIL,
		SPIRV,
		GLSL,
		GSSL,
		MSL
	};


	class SRK_FW_DLL ProgramSource : public Ref {
	public:
		ProgramSource();
		ProgramSource(ProgramSource&& value);

		ProgramLanguage language;
		ProgramStage stage;
		std::string version;
		std::string entryPoint;
		ByteArray data;

		ProgramSource& SRK_CALL operator=(ProgramSource&& value) noexcept;

		bool SRK_CALL isValid() const;

		inline static std::string SRK_CALL toHLSLShaderModel(const ProgramSource& source) {
			return toHLSLShaderModel(source.stage, source.version);
		}

		static std::string SRK_CALL toHLSLShaderModel(ProgramStage stage, const std::string_view& version);

		inline static std::string SRK_CALL getEntryPoint(const ProgramSource& source) {
			return getEntryPoint(source.entryPoint);
		}
		inline static std::string SRK_CALL getEntryPoint(const std::string_view& entryPoint) {
			return entryPoint.empty() ? "main" : std::move(std::string(entryPoint));
		}
	};
}