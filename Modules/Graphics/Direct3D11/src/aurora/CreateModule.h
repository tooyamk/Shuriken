#pragma once

#include "Graphics.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		d3d11::Graphics::CreateConfig conf;
		conf.loader = loader;

		if (!args) {
			printdln("DX11GraphicsModule create error : no args");
			return nullptr;
		}

		conf.app = (IApplication*)args->tryGet("app").toNumber<uintptr_t>();
		if (!conf.app) {
			printdln("DX11GraphicsModule create error : no app");
			return nullptr;
		}

		conf.adapter = (GraphicsAdapter*)args->tryGet("adapter").toNumber<uintptr_t>();
		conf.sampleCount = args->tryGet("sampleCount").toNumber<SampleCount>(1);
		conf.debug = args->tryGet("debug").toBool();
		conf.driverType = args->tryGet("driverType").toStringView();

		auto g = new d3d11::Graphics();
		if (!g->createDevice(conf)) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif