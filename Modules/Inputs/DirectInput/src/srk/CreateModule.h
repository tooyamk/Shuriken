#pragma once

#include "Input.h"
#include "srk/Debug.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateInputModuleDesc& desc) {
		using namespace std::literals;

		if (!desc.window) {
			printaln(L"DirectInputModule create error : no window"sv);
			return nullptr;
		}

		return new direct_input::Input(loader, desc);
	}
}
#endif