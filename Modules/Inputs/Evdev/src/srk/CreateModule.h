#pragma once

#include "Input.h"
#include "srk/Printer.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateInputModuleDescriptor& desc) {
		using namespace std::literals;

		if (!desc.window) {
			printaln(L"evdevModule create error : no window"sv);
			return nullptr;
		}

		return new evdev_input::Input(loader, desc);
	}
}
#endif