#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(const Args* args) {
		if (!args) {
			println("GlewGraphicsModule create error : no args");
			return nullptr;
		}

		auto app = args->get<Application*>("app", nullptr);
		if (!app) println("GlewGraphicsModule create error : no app");

		auto g = new win_glew::Graphics(app, args->get<IProgramSourceTranslator*>("trans", nullptr));
		if (!g->createDevice(args->get<const GraphicsAdapter*>("adapter", nullptr))) {
			g->unref();
			g = nullptr;
		}

		return g;
	}
}
#endif