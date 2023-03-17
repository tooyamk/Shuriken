#include "Application.h"

#if __has_include(<sys/ptrace.h>)
#	define SRK_HAS_SYS_PTRACE_H
#	include <sys/ptrace.h>
#endif

#if __has_include(<sys/sysctl.h>)
#    define SRK_HAS_SYS_SYSCTL_H
#    include <sys/sysctl.h>
#endif

namespace srk {
	bool Application::isDebuggerAttached() {
#if SRK_OS == SRK_OS_WINDOWS
		return ::IsDebuggerPresent();
#elif SRK_OS == SRK_OS_MACOS
#   if defined(SRK_HAS_SYS_SYSCTL_H)
		int32_t mib[4];
		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PID;
		mib[3] = Application::getCurrentProcessId();

		kinfo_proc info;
		info.kp_proc.p_flag = 0;

		auto size = sizeof(info);
		::sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0);
		return (info.kp_proc.p_flag & P_TRACED) != 0;
#   endif
#elif defined(SRK_HAS_SYS_PTRACE_H)
#	ifdef PT_DENY_ATTACH
		return ::ptrace(PT_DENY_ATTACH, 0, 0, 0) < 0;
#	else
		return ::ptrace(PTRACE_TRACEME, 0, 0, 0) < 0;
#	endif
#else
		return false;
#endif
	}

    std::filesystem::path Application::getAppPath() {
#if SRK_OS == SRK_OS_WINDOWS
		wchar_t path[FILENAME_MAX];
		auto count = GetModuleFileNameW(nullptr, path, FILENAME_MAX);
		return std::filesystem::path(std::wstring_view(path, count > 0 ? count : 0));
#elif SRK_OS == SRK_OS_LINUX
		char path[FILENAME_MAX];
		auto count = readlink("/proc/self/exe", path, FILENAME_MAX);
		return std::filesystem::path(std::string_view(path, count > 0 ? count : 0));
#else
		return std::filesystem::path(std::string_view());
#endif
    }
}