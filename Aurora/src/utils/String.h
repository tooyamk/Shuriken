#pragma once

#include "base/LowLevel.h"
#include <string>

namespace aurora {
	class AE_DLL String {
	public:
		static void AE_CALL calcUnicodeToUtf8Length(const wchar_t* in, ui32 inLen, ui32& unicodeLen, ui32& utf8Len);
		static i32 AE_CALL UnicodeToUtf8(const wchar_t* in, ui32 inLen, char* out, ui32 outLen);
		static std::string AE_CALL UnicodeToUtf8(const std::wstring& in);
		static void AE_CALL calcUtf8ToUnicodeLength(const i8* in, ui32 inLen, ui32& utf8Len, ui32& unicodeLen);
		static i32 AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen);
		static std::wstring AE_CALL Utf8ToUnicode(const std::string& in);
		static i32 AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t*& out);

	private:
		static ui32 AE_CALL _UnicodeToUtf8(const wchar_t * in, ui32 inLen, char* out);
		static ui32 AE_CALL _Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out);
	};
}