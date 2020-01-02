#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const Args* args) {
		if (!args) {
			println("DX11GraphicsModule create error : no args");
			return nullptr;
		}

		auto app = args->get<Application*>("app");
		if (!app && !app.value()) {
			println("DX11GraphicsModule create error : no app");
			return nullptr;
		}

		auto g = new win_d3d11::Graphics(loader, app.value());
		auto adapter = args->get<const GraphicsAdapter*>("adapter");
		if (!g->createDevice(adapter.has_value() ? adapter.value() : nullptr)) {
			g->unref();
			g = nullptr;
		}

		return g;
	}
}
#endif