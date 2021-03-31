#pragma once

#if AE_OS == AE_OS_WIN
#include "windows/Input.h"
#else
#include "Input.h"
#endif

#ifdef AE_MODULE_EXPORTS
namespace aurora::modules::inputs {
	extern "C" AE_MODULE_DLL_EXPORT void* AE_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		return new generic_input::Input(loader);
	}
}
#endif