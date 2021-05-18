#include "DynamicLib.h"
#include "aurora/String.h"

#if AE_OS != AE_OS_WINDOWS
#include <dlfcn.h>
#endif

namespace aurora {
	DynamicLib::DynamicLib() :
		_lib(nullptr) {
	}

	DynamicLib::~DynamicLib() {
		release();
	}

	bool DynamicLib::_load(const std::string_view& path) {
		release();
#if AE_OS == AE_OS_WINDOWS
		auto wpath = String::Utf8ToUnicode(path);
		if (wpath.empty()) return false;
		_lib = LoadLibraryW(wpath.data());
#else
		_lib = dlopen(path.data(), RTLD_LAZY);
#endif
		return _lib;
	}

	void DynamicLib::release() {
		if (_lib) {
#if AE_OS == AE_OS_WINDOWS
			FreeLibrary((HMODULE)_lib);
#else
			dlclose(_lib);
#endif
			_lib = nullptr;
		}
	}

	void* DynamicLib::getSymbolAddress(const std::string_view& name) const {
		if (_lib) {
#if AE_OS == AE_OS_WINDOWS
			return GetProcAddress((HMODULE)_lib, name.data());
#else
			return dlsym(_lib, name.data());
#endif
		} else {
			return nullptr;
		}
	}
}