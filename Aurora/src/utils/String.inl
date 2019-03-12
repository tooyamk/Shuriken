#include "String.h"
#include <codecvt>

AE_NS_BEGIN

std::string String::UnicodeToUTF8(const wchar_t* src) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(src);
}

std::wstring String::UTF8ToUnicode(const i8* src) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(src);
}

AE_NS_END