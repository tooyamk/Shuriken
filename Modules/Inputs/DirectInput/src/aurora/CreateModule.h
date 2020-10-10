#pragma once

#include "Input.h"
#include "aurora/Debug.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::inputs {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		if (!args) {
			printdln("DirectInputModule create error : no args");
			return nullptr;
		}

		auto app = (IApplication*)args->tryGet("app").toNumber<uintptr_t>();
		if (!app) {
			printdln("DirectInputModule create error : no app");
			return nullptr;
		}

		return new direct_input::Input(loader, app);
	}
}
#endif