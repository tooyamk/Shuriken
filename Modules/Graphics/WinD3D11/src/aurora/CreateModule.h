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
		if (!app && !*app) {
			println("DX11GraphicsModule create error : no app");
			return nullptr;
		}

		auto g = new win_d3d11::Graphics();
		auto adapter = args->get<const GraphicsAdapter*>("adapter");
		auto sc = args->get<SampleCount>("sampleCount");
		if (!g->createDevice(loader, *app, adapter ? *adapter : nullptr, sc ? *sc : 1)) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif