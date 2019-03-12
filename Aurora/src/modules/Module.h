#pragma once

#include "base/DynamicLib.h"
#include "base/Ref.h"

typedef void*(*AE_CREATE_MODULE_FUN)();

AE_NS_BEGIN

class AE_DLL Module : public Ref {
public:
	Module() : _createFn(nullptr) {}
	virtual ~Module() {}

	bool AE_CALL load(const i8* path) {
		if (_lib.isLoaded()) _lib.free();
		if (_lib.load(path)) {
			_createFn = (AE_CREATE_MODULE_FUN)_lib.getSymbolAddress("createModule");
			return _createFn;
		} else {
			_createFn = nullptr;
			return false;
		}
		return false;
	}

	void AE_CALLfree() {
		_lib.free();
	}

	template<class T>
	T* AE_CALL create() const {
		if (_createFn && _lib.isLoaded()) {
			return (T*)_createFn();
		} else {
			return nullptr;
		}
	}

private:
	DynamicLib _lib;
	AE_CREATE_MODULE_FUN _createFn;
};

AE_NS_END