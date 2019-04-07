#pragma once

#include <dxc/Support/Global.h>
#include <dxc/Support/Unicode.h>
//#include <dxc/Support/WinAdapter.h>
#include <dxc/Support/WinIncludes.h>

#include "dxc/dxcapi.h"

#include "modules/IGraphicsModule.h"
#include "base/DynamicLib.h"

#ifdef AE_X64
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
		ProgramSourceTranslator(const i8* dxc);
		virtual ~ProgramSourceTranslator();
		
		virtual ProgramSource AE_CALL translate(const ProgramSource& source, ProgramLanguage targetLanguage, const std::string& targetVersion) override;

	private:
		DynamicLib _dxcDll;

		CComPtr<IDxcLibrary> _dxcLib;
		CComPtr<IDxcCompiler> _dxcompiler;

		void _spirvTo(const ProgramSource& source, const i8* sourceData, ui32 sourceDataSize, 
			ProgramLanguage targetLanguage, const std::string& targetVersion, ProgramSource& dst);
	};
}

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(const ModuleArgs* args) {
		if (!args) {
			println("Module create err, no args");
			return nullptr;
		}

		auto dxc = args->get<const i8*>("dxc", nullptr);
		if (!dxc) println("Module create err, no dxc");

		return new program_source_translator::ProgramSourceTranslator(dxc);
	}
}
#endif