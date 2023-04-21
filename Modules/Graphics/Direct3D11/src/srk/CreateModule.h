#pragma once

#include "Graphics.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::graphics {
	extern "C" SRK_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateGrahpicsModuleDescriptor& desc) {
		auto g = new d3d11::Graphics();
		if (!g->createDevice(loader, desc)) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif