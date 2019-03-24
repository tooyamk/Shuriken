#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT void* createModule() {
	return new aurora::modules::graphics::win::glew::Graphics();
}
#endif