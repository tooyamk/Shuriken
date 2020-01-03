#pragma once

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
//#include "dxc/Support/WinAdapter.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/dxcapi.h"

#include "modules/graphics/IProgramSourceTranslator.h"
#include "base/DynamicLib.h"

#if AE_OS_BIT == AE_OS_BIT_64
#ifdef AE_DEBUG
#pragma comment(lib, "libs/spirv-cross/x64/spirv-cross-cored.lib")
#pragma comment(lib, "libs/spirv-cross/x64/spirv-cross-glsld.lib")
#pragma comment(lib, "libs/spirv-cross/x64/spirv-cross-msld.lib")
#else
#pragma comment(lib, "libs/spirv-cross/x64/spirv-cross-core.lib")
#pragma comment(lib, "libs/spirv-cross/x64/spirv-cross-glsl.lib")
#pragma comment(lib, "libs/spirv-cross/x64/spirv-cross-msl.lib")
#endif
#endif

namespace aurora::modules::graphics::program_source_translator {
	class ProgramSourceTranslator : public IProgramSourceTranslator {
	public:
		ProgramSourceTranslator(Ref* loader, const std::string_view& dxc);
		virtual ~ProgramSourceTranslator();

		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string_view& targetVersion) override;

	private:
		RefPtr<Ref> _loader;
		DynamicLib _dxcDll;

		CComPtr<IDxcLibrary> _dxcLib;
		CComPtr<IDxcCompiler> _dxcompiler;

		void _spirvTo(const ProgramSource& source, const uint8_t* sourceData, uint32_t sourceDataSize,
			ProgramLanguage targetLanguage, const std::string_view& targetVersion, ProgramSource& dst);
	};
}

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref * loader, const Args * args) {
		if (!args) {
			println("Module create err, no args");
			return nullptr;
		}

		auto dxc = args->get<const std::string>("dxc");
		if (!dxc) {
			println("Module create err, no dxc");
			return nullptr;
		}

		return new program_source_translator::ProgramSourceTranslator(loader, dxc.value().data());
	}
}
#endif