#pragma once

#include "Graphics.h"
#include "srk/Debug.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::graphics {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		vulkan::Graphics::CreateConfig conf;
		conf.loader = loader;

		if (!args) {
			printaln(L"DX11GraphicsModule create error : no args"sv);
			return nullptr;
		}

		conf.win = (decltype(conf.win))args->tryGet("win").toNumber<uintptr_t>();
		conf.transpiler = (decltype(conf.transpiler))args->tryGet("transpiler").toNumber<uintptr_t>();
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