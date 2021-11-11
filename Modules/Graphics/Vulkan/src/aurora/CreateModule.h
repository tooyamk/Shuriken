#pragma once

#include "Graphics.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		vulkan::Graphics::CreateConfig conf;
		conf.loader = loader;

		if (!args) {
			printaln(L"DX11GraphicsModule create error : no args"sv);
			return nullptr;
		}

		conf.app = (decltype(conf.app))args->tryGet("app").toNumber<uintptr_t>();
		conf.adapter = (decltype(conf.adapter))args->tryGet("adapter").toNumber<uintptr_t>();
		conf.sampleCount = args->tryGet("sampleCount").toNumber<decltype(conf.sampleCount)>(1);
		conf.debug = args->tryGet("debug").toBool();
		conf.offscreen = args->tryGet("offscreen").toBool();
		conf.driverType = args->tryGet("driverType").toStringView();
		conf.createProcessInfoHandler = (decltype(conf.createProcessInfoHandler))args->tryGet("createProcessInfoHandler").toNumber<uintptr_t>();

		auto g = new vulkan::Graphics();
		if (!g->createDevice(conf)) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif