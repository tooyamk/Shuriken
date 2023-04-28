#pragma once

#include "srk/Core.h"

#ifdef SRK_FW_EXPORTS
#	define SRK_FW_DLL SRK_DLL_EXPORT
#else
#	define SRK_FW_DLL SRK_DLL_IMPORT
#endif

#ifdef SRK_MODULE_EXPORTS
#	define SRK_MODULE_DLL SRK_DLL_EXPORT
#else
#	define SRK_MODULE_DLL SRK_DLL_IMPORT
#endif

#define SRK_CREATE_MODULE_FN_NAME srk_create_module