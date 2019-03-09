#pragma once

#include "base/Aurora.h"

AE_NS_BEGIN

class DynamicLib {
public:
	DynamicLib();
	~DynamicLib();

	bool load(const i8* path);
	void free();
	void* getSymbolAddress(const i8* name) const;

private:
	void* _lib;
};

AE_NS_END