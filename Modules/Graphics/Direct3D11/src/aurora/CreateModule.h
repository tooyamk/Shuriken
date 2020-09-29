#pragma once

#include "Graphics.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		if (!args) {
			printdln("DX11GraphicsModule create error : no args");
			return nullptr;
		}

		auto app = (IApplication*)args->tryGet("app").toNumber<uint64_t>();
		if (!app) {
			printdln("DX11GraphicsModule create error : no app");
			return nullptr;
		}

		auto adapter = (const GraphicsAdapter*)args->tryGet("adapter").toNumber<uint64_t>();
		auto sc = args->tryGet("sampleCount").toNumber<SampleCount>(1);

		auto g = new d3d11::Graphics();
		if (!g->createDevice(loader, app, adapter, sc, args->tryGet("debug").toBool())) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif