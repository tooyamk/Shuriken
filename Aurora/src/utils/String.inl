#include "String.h"
#include <codecvt>

AE_NS_BEGIN

std::string String::UnicodeToUTF8(const wi8* src) {
	std::wstring_convert<std::codecvt_utf8<wi8>>().to_bytes(src);
}

AE_NS_END