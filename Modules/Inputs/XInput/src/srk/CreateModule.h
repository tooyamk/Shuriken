#pragma once

#include "Input.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_MODULE_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateInputModuleDesc& desc) {
		return new xinput::Input(loader, desc);
	}
}
#endif