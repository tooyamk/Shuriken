#pragma once

#include "srk/Global.h"
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

        static std::filesystem::path SRK_CALL getAppPath();
    };
}