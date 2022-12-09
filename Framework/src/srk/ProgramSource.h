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

		inline static const std::string defaultEntryPoint = std::string("main");

		ProgramSource& SRK_CALL operator=(ProgramSource&& value) noexcept;

		bool SRK_CALL isValid() const;

		inline std::string_view toHLSLShaderStage() const {
			return toHLSLShaderStage(stage);
		}

		static std::string_view toHLSLShaderStage(ProgramStage stage);

		inline std::string SRK_CALL toHLSLShaderModel() const {
			return toHLSLShaderModel(stage, version);
		}

		static std::string SRK_CALL toHLSLShaderModel(ProgramStage stage, const std::string_view& version);

		inline const std::string& SRK_CALL getEntryPoint() const {
			return getEntryPoint(entryPoint);
		}
		inline static std::string_view SRK_CALL getEntryPoint(const std::string_view& entryPoint) {
			return entryPoint.empty() ? defaultEntryPoint : entryPoint;
		}
		inline static const std::string& SRK_CALL getEntryPoint(const std::string& entryPoint) {
			return entryPoint.empty() ? defaultEntryPoint : entryPoint;
		}
	};
}