#pragma once

#include "Input.h"

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::inputs {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(const Args* args) {
		if (!args) {
			println("DirectInputModule create error : no args");
			return nullptr;
		}

		auto app = args->get<Application*>("app", nullptr);
		if (!app) println("DirectInputModule create error : no app");

		return new win_direct_input::Input(app);
	}
}
#endif