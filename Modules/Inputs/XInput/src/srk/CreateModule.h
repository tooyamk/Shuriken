#pragma once

#include "Input.h"

#ifdef SRK_MODULE_EXPORTS
namespace srk::modules::inputs {
	extern "C" SRK_DLL_EXPORT void* SRK_CREATE_MODULE_FN_NAME(Ref* loader, const CreateInputModuleDescriptor& desc) {
		return new xinput::Input(loader, desc);
	}
}
#endif