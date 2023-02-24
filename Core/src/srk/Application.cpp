#include "Application.h"

namespace srk {
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