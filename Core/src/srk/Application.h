#pragma once

#include "srk/Core.h"
#include <filesystem>

namespace srk {
    class SRK_CORE_DLL Application {
    public:
        Application() = delete;

        inline static uint32_t SRK_CALL getCurrentProcessId() {
#if SRK_OS == SRK_OS_WINDOWS
		return ::GetCurrentProcessId();
#else
		return ::getpid();
#endif
	    }

        static bool SRK_CALL isDebuggerAttached();

        static std::filesystem::path SRK_CALL getAppPath();
    };
}