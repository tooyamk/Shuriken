#pragma once

#include "Manager.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::windows {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const SerializableObject* args) {
		return new windows_classic::Manager(loader);
	}
}
#endif