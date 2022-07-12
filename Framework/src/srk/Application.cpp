#include "Application.h"

#if SRK_OS != SRK_OS_WINDOWS
#	include <dlfcn.h>
#	include <signal.h>
#endif

namespace srk {
	Application::Application(const std::string_view& appId, uint32_t pid, const std::filesystem::path& appPath) :
		_appId(appId),
		_pid(_pid),
		_path(appPath.empty() ? srk::getAppPath() : appPath) {
		if (!_pid) {
#if SRK_OS == SRK_OS_WINDOWS
			_pid = GetCurrentProcessId();
			_native = GetModuleHandleW(nullptr);
#else
			_pid = getpid();
			_native = dlopen(nullptr);
#endif
		}
	}

	void Application::terminate(int32_t code) const {
#if SRK_OS == SRK_OS_WINDOWS
		if (auto p = OpenProcess(PROCESS_TERMINATE, false, _pid); p) TerminateProcess(p, code);
#else
		kill(_pid, SIGKILL);
#endif
	}
}