#pragma once

#include "Graphics.h"
#include "srk/Debug.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::graphics {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		gl::Graphics::CreateConfig conf;
		conf.loader = loader;

		if (!args) {
			printaln(L"GlewGraphicsModule create error : no args"sv);
			return nullptr;
		}

		conf.win = (decltype(conf.win))args->tryGet("win").toNumber<uintptr_t>();
		if (!conf.win) {
			printaln(L"GlewGraphicsModule create error : no window"sv);
			return nullptr;
		}

		conf.transpiler = (decltype(conf.transpiler))args->tryGet("transpiler").toNumber<uintptr_t>();
		conf.adapter = (decltype(conf.adapter))args->tryGet("adapter").toNumber<uintptr_t>();
		conf.sampleCount = args->tryGet("sampleCount").toNumber<decltype(conf.sampleCount)>(1);
		conf.debug = args->tryGet("debug").toBool();

		auto g = new gl::Graphics();
		if (!g->createDevice(conf)) {
			Ref::unref(*g);
			g = nullptr;
		}

		return g;
	}
}
#endif