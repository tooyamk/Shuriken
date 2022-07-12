#include "Application.h"

#if SRK_OS != SRK_OS_WINDOWS
#	include <dlfcn.h>
#endif

namespace srk {
	Application::Application(const std::string_view& appId, void* native, const std::filesystem::path& appPath) :
		_appId(appId),
		_native(native),
		_path(appPath.empty() ? srk::getAppPath() : appPath) {
		if (!_native) {
#if SRK_OS == SRK_OS_WINDOWS
			_native = GetModuleHandleW(nullptr);
#else
			_native = dlopen(nullptr);
#endif
		}
	}
}