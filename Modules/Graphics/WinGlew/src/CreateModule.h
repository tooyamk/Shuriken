#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT aurora::modules::GraphicsModule* AE_CREATE_MODULE_FN_NAME(aurora::Application* app) {
	if (!app) {
		aurora::println("Module create err, app is null");
		return nullptr;
	}
	return new aurora::modules::graphics_win_glew::Graphics(app);
}
#endif