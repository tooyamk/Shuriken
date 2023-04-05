#pragma once

#include "Graphics.h"
#include "srk/Printer.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::graphics {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateGrahpicsModuleDescriptor& desc) {
		using namespace std::string_view_literals;

		if (!desc.window) {
			printaln(L"GlewGraphicsModule create error : no window"sv);
			return nullptr;
		}

		auto g = new gl::Graphics();
		if (!g->createDevice(loader, desc)) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif