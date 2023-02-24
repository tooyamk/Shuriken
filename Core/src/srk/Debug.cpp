#include "Debug.h"
#if SRK_OS == SRK_OS_MACOS
#	include "srk/Application.h"
#endif

#if __has_include(<android/log.h>)
#	define SRK_HAS_ANDROID_LOG_H
#	include <android/log.h>
#endif

#if __has_include(<sys/ptrace.h>)
#	define SRK_HAS_SYS_PTRACE_H
#	include <sys/ptrace.h>
#endif

#if __has_include(<sys/sysctl.h>)
#    define SRK_HAS_SYS_SYSCTL_H
#    include <sys/sysctl.h>
#endif

namespace srk {
	bool Debug::isDebuggerAttached() {
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

	bool Debug::DebuggerOutput::operator()(const std::wstring_view& data) const {
		if (isDebuggerAttached()) {
#if SRK_OS == SRK_OS_WINDOWS
			::OutputDebugStringW(data.data());
			return true;
#elif SRK_OS == SRK_OS_ANDROID
#	ifdef SRK_HAS_ANDROID_LOG_H
			__android_log_print(ANDROID_LOG_INFO, "Shuriken", "%ls", data.data());
			return true;
#	endif
#endif
		}

		return false;
	}
}