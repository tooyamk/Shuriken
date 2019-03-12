#pragma once

#include "base/Aurora.h"

AE_NS_BEGIN

class AE_DLL DynamicLib {
public:
	DynamicLib();
	virtual ~DynamicLib();

	inline bool isLoaded() const { return _lib; }
	bool load(const i8* path);
	void free();
	void* getSymbolAddress(const i8* name) const;

private:
	void* _lib;
};

AE_NS_END