#pragma once

#include "base/Aurora.h"

AE_NS_BEGIN

class AE_DLL DynamicLib {
public:
	DynamicLib();
	virtual ~DynamicLib();

	inline bool AE_CALL isLoaded() const { return _lib; }
	bool AE_CALL load(const i8* path);
	void AE_CALL free();
	void* AE_CALL getSymbolAddress(const i8* name) const;

private:
	void* _lib;
};

AE_NS_END