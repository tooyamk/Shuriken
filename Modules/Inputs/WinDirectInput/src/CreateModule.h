#pragma once

#include "DirectInput.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(const ModuleArgs* args) {
		if (!args) {
			println("Module create err, no args");
			return nullptr;
		}

		auto app = args->get<Application*>("app", nullptr);
		if (!app) println("Module create err, no app");

		return new win_direct_input::DirectInput(app);
	}
}
#endif