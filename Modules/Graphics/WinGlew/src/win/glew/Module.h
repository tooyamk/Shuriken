#pragma once

#include "Graphics.h"

#ifdef AE_MODULE_EXPORTS
extern "C" AE_MODULE_DLL_EXPORT aurora::modules::GraphicsModule* AE_CREATE_MODULE_FN_NAME() {
	return new aurora::modules::graphics::win::glew::Graphics();
}
#endif