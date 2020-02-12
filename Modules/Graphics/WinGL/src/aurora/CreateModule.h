#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const Args* args) {
		if (!args) {
			println("GlewGraphicsModule create error : no args");
			return nullptr;
		}

		auto app = args->get<Application*>("app");
		if (!app && !app.value()) {
			println("GlewGraphicsModule create error : no app");
			return nullptr;
		}

		auto trans = args->get<IProgramSourceTranslator*>("trans");
		auto adapter = args->get<const GraphicsAdapter*>("adapter");
		auto sc = args->get<SampleCount>("sampleCount");
		auto g = new win_gl::Graphics(loader, app.value(), trans.has_value() ? trans.value() : nullptr);
		if (!g->createDevice(adapter.has_value() ? adapter.value() : nullptr, sc ? *sc : 1)) {
			g->unref();
			g = nullptr;
		}

		return g;
	}
}
#endif