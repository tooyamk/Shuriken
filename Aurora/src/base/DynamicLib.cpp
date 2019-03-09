#include "DynamicLib.h"

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC
#include <dlfcn.h>
#endif

AE_NS_BEGIN

DynamicLib::DynamicLib() :
	_lib(nullptr) {
}

DynamicLib::~DynamicLib() {
	free();
}

bool DynamicLib::load(const i8* path) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN32
	_lib = LoadLibraryA(path);
#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC
	_lib = dlopen(path, RTLD_LAZY);
#endif
	return _lib;
}

void DynamicLib::free() {
	if (_lib) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN32
		FreeLibrary((HMODULE)_lib);
#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC
		dlclose(_lib);
#endif
		_lib = nullptr;
	}
}

void* DynamicLib::getSymbolAddress(const i8* name) const {
	if (_lib) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN32
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

AE_NS_END