#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(const Args* args) {
		if (!args) {
			println("Module create err, no args");
			return nullptr;
		}

		auto app = args->get<Application*>("app", nullptr);
		if (!app) println("Module create err, no app");

		auto g = new win_d3d11::Graphics(app);
		if (!g->createDevice(args->get<const GraphicsAdapter*>("adapter", nullptr))) {
			g->unref();
			g = nullptr;
		}

		return g;
	}
}
#endif