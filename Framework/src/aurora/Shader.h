#pragma once

#include "aurora/ProgramSource.h"
#include "aurora/ShaderDefine.h"
#include "aurora/hash/CRC.h"
#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora::modules::graphics {
	class IGraphicsModule;
	class IProgram;
}

namespace aurora {
	class ShaderDefine;
	class IShaderDefineGetter;


	class AE_FW_DLL Shader : public Ref {
	public:
		using IncludeHandler = std::function<ByteArray(const Shader&, ProgramStage, const std::string_view&)>;

		Shader();

		void AE_CALL set(modules::graphics::IGraphicsModule* graphics, ProgramSource* vs, ProgramSource* ps, 
			const ShaderDefine* staticDefines, size_t numStaticDefines, const std::string_view* dynamicDefines, size_t numDynamicDefines,
			const IncludeHandler& handler);
		modules::graphics::IProgram* AE_CALL select(const IShaderDefineGetter* getter);

		void AE_CALL unset();

		void AE_CALL setVariant(ProgramSource* vs, ProgramSource* ps, const IShaderDefineGetter* getter);

	protected:
		inline static auto _crcTable = hash::CRC::createTable<64>(0x42F0E1EBA9EA3693ULL);


		struct Variant {
			RefPtr<ProgramSource> vs;
			RefPtr<ProgramSource> ps;
		};


		RefPtr<modules::graphics::IGraphicsModule> _graphics;
		RefPtr<ProgramSource> _vs;
		RefPtr<ProgramSource> _ps;
		IncludeHandler _includeHhandler;

		std::unordered_map<uint64_t, Variant> _variants;

		std::vector<std::string> _staticDefines;
		std::vector<std::string> _dynamicDefines;
		std::vector<ShaderDefine> _defines;

		std::unordered_map<uint64_t, RefPtr<modules::graphics::IProgram>> _programs;
	};
}