#pragma once

#include "base/Aurora.h"
#include <string>

namespace aurora {
	class AE_DLL String {
	public:
		static int AE_CALL UnicodeToUtf8(const wchar_t * in, ui32 inLen, char* out, ui32 outLen);
		static int AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen);
	};
}