#pragma once

#include "Graphics.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		if (!args) {
			printdln("GlewGraphicsModule create error : no args");
			return nullptr;
		}

		auto app = (IApplication*)args->tryGet("app").toNumber<uint64_t>();
		if (!app) {
			printdln("GlewGraphicsModule create error : no app");
			return nullptr;
		}

		auto trans = (IProgramSourceTranslator*)args->tryGet("trans").toNumber<uint64_t>();
		auto adapter = (const GraphicsAdapter*)args->tryGet("adapter").toNumber<uint64_t>();
		auto sc = args->tryGet("sampleCount").toNumber<SampleCount>(1);

		auto g = new win_gl::Graphics();
		if (!g->createDevice(loader, app, trans, adapter, sc, args->tryGet("debug").toBool())) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif