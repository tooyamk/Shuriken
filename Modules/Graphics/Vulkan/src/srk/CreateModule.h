#pragma once

#include "Graphics.h"
#include "srk/Printer.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::graphics {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateGrahpicsModuleDesc& desc) {
		auto g = new vulkan::Graphics();
		if (!g->createDevice(loader, desc)) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif