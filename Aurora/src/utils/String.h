#pragma once

#include "base/Aurora.h"
#include <string>

AE_NS_BEGIN

class AE_DLL String {
public:
	inline static std::string AE_CALL UnicodeToUTF8(const wchar_t* src);
	inline static std::wstring AE_CALL UTF8ToUnicode(const i8* src);
};

AE_NS_END

#include "String.inl"