#include "String.h"

namespace aurora {
	bool String::isUTF8(const char* data, size_t len) {
		const auto buf = (uint8_t*)data;
		size_t i = 0;
		while (i < len) {
			if (auto c = buf[i]; c < 0x80) {
				++i;
			} else if (c < 0xC0) {
				return false;
			} else if (c < 0xE0) {
				if (i >= len - 1) break;
				if ((buf[i + 1] & 0xC0) != 0x80) return false;
				i += 2;
			} else if (c < 0xF0) {
				if (i >= len - 2) break;
				if ((buf[i + 1] & 0xC0) != 0x80 || (buf[i + 2] & 0xC0) != 0x80) return false;
				i += 3;
			} else {
				return false;
			}
		}
		return true;
	}

	std::string::size_type String::_UnicodeToUtf8(const wchar_t* in, std::wstring::size_type inLen, char* out) {
		std::wstring::size_type s = 0;
		std::string::size_type d = 0;

		while (s < inLen) {
			if (wchar_t c = in[s++]; c < 0x80) {  //
				//length = 1;
				out[d++] = (char)c;
			} else if (c < 0x800) {
				//length = 2;
				out[d++] = 0xC0 | (c >> 6);
				out[d++] = 0x80 | (c & 0x3F);
			} else if (c < 0x10000) {
				//length = 3;
				out[d++] = 0xE0 | (c >> 12);
				out[d++] = 0x80 | ((c >> 6) & 0x3F);
				out[d++] = 0x80 | (c & 0x3F);
			} else if (c < 0x200000) {
				//length = 4;
				out[d++] = 0xF0 | ((int)c >> 18);
				out[d++] = 0x80 | ((c >> 12) & 0x3F);
				out[d++] = 0x80 | ((c >> 6) & 0x3F);
				out[d++] = 0x80 | (c & 0x3F);
			}
		}

		return d;
	}

	std::wstring::size_type String::_Utf8ToUnicode(const char* in, std::string::size_type inLen, wchar_t* out) {
		std::string::size_type s = 0;
		std::wstring::size_type d = 0;

		while (s < inLen) {
			if (uint8_t c = in[s]; (c & 0x80) == 0) {
				out[d++] = in[s++];
			} else if ((c & 0xE0) == 0xC0) {// 110x-xxxx 10xx-xxxx
				out[d++] = ((c & 0x3F) << 6) | (in[s + 1] & 0x3F);
				s += 2;
			} else if ((c & 0xF0) == 0xE0) {// 1110-xxxx 10xx-xxxx 10xx-xxxx
				out[d++] = ((c & 0x1F) << 12) | ((in[s + 1] & 0x3F) << 6) | (in[s + 2] & 0x3F);
				s += 3;
			} else if ((c & 0xF8) == 0xF0) {// 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
				out[d++] = ((in[s + 1] & 0x3F) << 12) | ((in[s + 2] & 0x3F) << 6) | (in[s + 3] & 0x3F);
				/*
				//wideChar = (in[s + 0] & 0x0F) << 18;
				wideChar = (in[s + 1] & 0x3F) << 12;
				wideChar |= (in[s + 2] & 0x3F) << 6;
				wideChar |= (in[s + 3] & 0x3F);
				*/
				s += 4;
			} else {// 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
				out[d++] = ((in[s + 2] & 0x3F) << 12) | ((in[s + 3] & 0x3F) << 6) | (in[s + 4] & 0x3F);
				/*
				//wideChar = (in[s + 0] & 0x07) << 24;
				//wideChar = (in[s + 1] & 0x3F) << 18;
				wideChar = (in[s + 2] & 0x3F) << 12;
				wideChar |= (in[s + 3] & 0x3F) << 6;
				wideChar |= (in[s + 4] & 0x3F);
				*/
				s += 5;
			}
		}

		return d;
	}

	std::string String::toString(const uint8_t* value, size_t size) {
		std::string str(size << 1, 0);
		char buf[3];
		for (size_t i = 0; i < size; ++i) {
			snprintf(buf, sizeof(buf), "%02x", value[i]);
			size_t idx = i << 1;
			str[idx++] = buf[0];
			str[idx] = buf[1];
		}
		return std::move(str);
	}

	bool String::isEqual(const char* str1, const char* str2) {
		if (str1 == str2) return true;
		
		size_t i = 0;
		do {
			if (str1[i] != str2[i]) return false;
			if (str1[i] == 0) return true;
			++i;
		} while(true);
	}
}