#pragma once

#include "base/Aurora.h"
#include <string>

AE_NS_BEGIN

class AE_DLL String {
public:
	inline static std::string AE_CALL UnicodeToUTF8(const wi8* src);
};

AE_NS_END

#include "String.inl"