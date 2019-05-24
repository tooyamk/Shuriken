#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const Args* args) {
		if (!args) {
			println("DX11GraphicsModule create error : no args");
			return nullptr;
		}

		auto app = args->get<Application*>("app", nullptr);
		if (!app) println("DX11GraphicsModule create error : no app");

		auto g = new win_d3d11::Graphics(loader, app);
		if (!g->createDevice(args->get<const GraphicsAdapter*>("adapter", nullptr))) {
			g->unref();
			g = nullptr;
		}

		return g;
	}
}
#endif