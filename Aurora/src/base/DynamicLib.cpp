#include "DynamicLib.h"
#include "utils/String.h"

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC
#include <dlfcn.h>
#endif

namespace aurora {
	DynamicLib::DynamicLib() :
		_lib(nullptr) {
	}

	DynamicLib::~DynamicLib() {
		free();
	}

	bool DynamicLib::load(const i8* path) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		wchar_t* out = nullptr;
		auto len = String::Utf8ToUnicode(path, 0xFFFFFFFFui32, out);
		if (len < 0) return false;
		_lib = LoadLibraryW(out);
		delete[] out;
#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC
		_lib = dlopen(path, RTLD_LAZY);
#endif
		return _lib;
	}

	void DynamicLib::free() {
		if (_lib) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
			FreeLibrary((HMODULE)_lib);
#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC
			dlclose(_lib);
#endif
			_lib = nullptr;
		}
	}

	void* DynamicLib::getSymbolAddress(const i8* name) const {
		if (_lib) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
			return GetProcAddress((HMODULE)_lib, name);
#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC
			return dlsym(_lib, name);
#else
			return nullptr;
#endif
		} else {
			return nullptr;
		}
	}
}