#include "DynamicLib.h"
#include "base/String.h"

#if AE_OS != AE_OS_WIN
#include <dlfcn.h>
#endif

namespace aurora {
	DynamicLib::DynamicLib() :
		_lib(nullptr) {
	}

	DynamicLib::~DynamicLib() {
		free();
	}

	bool DynamicLib::load(const char* path) {
#if AE_OS == AE_OS_WIN
		wchar_t* out = nullptr;
		if (String::Utf8ToUnicode(path, (std::numeric_limits<uint32_t>::max)(), out) == std::string::npos) return false;
		_lib = LoadLibraryW(out);
		delete[] out;
#else
		_lib = dlopen(path, RTLD_LAZY);
#endif
		return _lib;
	}

	void DynamicLib::free() {
		if (_lib) {
#if AE_OS == AE_OS_WIN
			FreeLibrary((HMODULE)_lib);
#else
			dlclose(_lib);
#endif
			_lib = nullptr;
		}
	}

	void* DynamicLib::getSymbolAddress(const char* name) const {
		if (_lib) {
#if AE_OS == AE_OS_WIN
			return GetProcAddress((HMODULE)_lib, name);
#else
			return dlsym(_lib, name);
#endif
		} else {
			return nullptr;
		}
	}
}