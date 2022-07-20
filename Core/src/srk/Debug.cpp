#include "Debug.h"

#if __has_include(<android/log.h>)
#	define SRK_HAS_ANDROID_LOG_H
#	include <android/log.h>
#endif

#if __has_include(<sys/ptrace.h>)
#	define SRK_HAS_SYS_PTRACE_H
#	include <sys/ptrace.h>
#endif

namespace srk {
	bool Debug::isDebuggerPresent() {
#if SRK_OS == SRK_OS_WINDOWS
		return ::IsDebuggerPresent();
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
		if (Debug::isDebuggerPresent()) {
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