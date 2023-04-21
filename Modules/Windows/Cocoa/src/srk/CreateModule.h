#pragma once

#include "Manager.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::windows {
	extern "C" SRK_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader) {
		return new cocoa::Manager(loader);
	}
}
#endif