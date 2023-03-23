#include "DynamicLibraryLoader.h"
#include "srk/String.h"

#if SRK_OS != SRK_OS_WINDOWS
#	include <dlfcn.h>
#endif

namespace srk {
	DynamicLibraryLoader::DynamicLibraryLoader() :
		_lib(nullptr) {
	}

	DynamicLibraryLoader::DynamicLibraryLoader(DynamicLibraryLoader&& other) noexcept :
		_lib(other._lib) {
		other._lib = nullptr;
	}

	DynamicLibraryLoader::~DynamicLibraryLoader() {
		release();
	}

	bool DynamicLibraryLoader::_load(const std::string_view& path) {
		release();
#if SRK_OS == SRK_OS_WINDOWS
		auto wpath = String::Utf8ToUnicode(path);
		if (wpath.empty()) return false;
		_lib = LoadLibraryW(wpath.data());
#else
		_lib = dlopen(path.data(), RTLD_LAZY);
#endif
		return _lib;
	}

	void DynamicLibraryLoader::release() {
		if (_lib) {
#if SRK_OS == SRK_OS_WINDOWS
			FreeLibrary((HMODULE)_lib);
#else
			dlclose(_lib);
#endif
			_lib = nullptr;
		}
	}

	void* DynamicLibraryLoader::getSymbolAddress(const std::string_view& name) const {
		if (_lib) {
#if SRK_OS == SRK_OS_WINDOWS
			return GetProcAddress((HMODULE)_lib, name.data());
#else
			return dlsym(_lib, name.data());
#endif
		} else {
			return nullptr;
		}
	}
}