#pragma once

#include "Input.h"
#include "srk/Printer.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateInputModuleDescriptor& desc) {
		using namespace std::string_view_literals;

		if (!desc.window) {
			printaln(L"X11Module create error : no window"sv);
			return nullptr;
		}

		return new x11::Input(loader, desc);
	}
}
#endif