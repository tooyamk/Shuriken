#pragma once

#include "Graphics.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::graphics {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		using namespace std::literals;

		gl::Graphics::CreateConfig conf;
		conf.loader = loader;

		if (!args) {
			printaln(L"GlewGraphicsModule create error : no args"sv);
			return nullptr;
		}

		conf.app = (decltype(conf.app))args->tryGet("app").toNumber<uintptr_t>();
		if (!conf.app) {
			printaln(L"GlewGraphicsModule create error : no app"sv);
			return nullptr;
		}

		conf.trans = (decltype(conf.trans))args->tryGet("trans").toNumber<uintptr_t>();
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