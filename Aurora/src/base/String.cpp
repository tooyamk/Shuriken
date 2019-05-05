#include "String.h"

namespace aurora {
	void String::calcUnicodeToUtf8Length(const wchar_t* in, ui32 inLen, ui32& unicodeLen, ui32& utf8Len) {
		ui32 s = 0, d = 0;
		if (in) {
			while (s < inLen) {
				wchar_t c = in[s++];
				if (c == 0) {
					break;
				} else if (c < 0x80) {  //
					//length = 1;
					++d;
				} else if (c < 0x800) {
					//length = 2;
					d += 2;
				} else if (c < 0x10000) {
					//length = 3;
					d += 3;
				} else if (c < 0x200000) {
					//length = 4;
					d += 4;
				}
			}
		}

		unicodeLen = s;
		utf8Len = d;
	}

	i32 String::UnicodeToUtf8(const wchar_t * in, ui32 inLen, char* out, ui32 outLen) {
		if (!in || !out) return -1;

		ui32 unicodeLen, utf8Len;
		calcUnicodeToUtf8Length(in, inLen, unicodeLen, utf8Len);
		if (outLen < unicodeLen) return -1;

		return _UnicodeToUtf8(in, unicodeLen, out);
	}

	std::string String::UnicodeToUtf8(const std::wstring& in) {
		ui32 unicodeLen, utf8Len;
		calcUnicodeToUtf8Length(in.c_str(), in.size(), unicodeLen, utf8Len);
		++utf8Len;
		auto out = new i8[utf8Len];
		auto len = _UnicodeToUtf8(in.c_str(), unicodeLen, out);
		out[len] = 0;

		std::string s(out);
		delete[] out;

		return std::move(s);
	}

	void String::calcUtf8ToUnicodeLength(const i8* in, ui32 inLen, ui32& utf8Len, ui32& unicodeLen) {
		ui32 s = 0, d = 0;
		if (in) {
			for (; s < inLen;) {
				ui8 c = in[s];
				if (c == 0) {
					break;
				} else if ((c & 0x80) == 0) {
					++s;
				} else if ((c & 0xE0) == 0xC0) {// 110x-xxxx 10xx-xxxx
					s += 2;
				} else if ((c & 0xF0) == 0xE0) {// 1110-xxxx 10xx-xxxx 10xx-xxxx
					s += 3;
				} else if ((c & 0xF8) == 0xF0) {// 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
					s += 4;
				} else {// 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
					s += 5;
				}
				++d;
			}
		}

		utf8Len = s;
		unicodeLen = d;
	}

	i32 String::Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen) {
		if (!in || !out) return -1;

		ui32 utf8Len, unicodeLen;
		calcUtf8ToUnicodeLength(in, inLen, utf8Len, unicodeLen);
		if (outLen < unicodeLen) return -1;

		return _Utf8ToUnicode(in, utf8Len, out);
	}

	std::wstring String::Utf8ToUnicode(const std::string& in) {
		ui32 utf8Len, unicodeLen;
		calcUtf8ToUnicodeLength(in.c_str(), in.size(), utf8Len, unicodeLen);
		++unicodeLen;
		auto out = new wchar_t[unicodeLen];
		auto len = _Utf8ToUnicode(in.c_str(), utf8Len, out);
		out[len] = 0;

		std::wstring s(out);
		delete[] out;

		return std::move(s);
	}

	i32 String::Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t*& out) {
		if (!in) return -1;

		ui32 utf8Len, unicodeLen;
		calcUtf8ToUnicodeLength(in, inLen, utf8Len, unicodeLen);
		++unicodeLen;
		out = new wchar_t[unicodeLen];
		auto len = _Utf8ToUnicode(in, utf8Len, out);
		out[len] = 0;

		return len;
	}

	ui32 String::_UnicodeToUtf8(const wchar_t* in, ui32 inLen, char* out) {
		ui32 s = 0, d = 0;
		while (s < inLen) {
			wchar_t c = in[s++];
			if (c < 0x80) {  //
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

	ui32 String::_Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out) {
		ui32 s = 0, d = 0;
		while (s < inLen) {
			ui8 c = in[s];
			if ((c & 0x80) == 0) {
				out[d++] = in[s++];
			} else if ((c & 0xE0) == 0xC0) {// 110x-xxxx 10xx-xxxx
				wchar_t& wideChar = out[d++];
				wideChar = (in[s + 0] & 0x3F) << 6;
				wideChar |= (in[s + 1] & 0x3F);

				s += 2;
			} else if ((c & 0xF0) == 0xE0) {// 1110-xxxx 10xx-xxxx 10xx-xxxx
				wchar_t& wideChar = out[d++];

				wideChar = (in[s + 0] & 0x1F) << 12;
				wideChar |= (in[s + 1] & 0x3F) << 6;
				wideChar |= (in[s + 2] & 0x3F);

				s += 3;
			} else if ((c & 0xF8) == 0xF0) {// 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
				wchar_t& wideChar = out[d++];

				//wideChar = (in[s + 0] & 0x0F) << 18;
				wideChar = (in[s + 1] & 0x3F) << 12;
				wideChar |= (in[s + 2] & 0x3F) << 6;
				wideChar |= (in[s + 3] & 0x3F);

				s += 4;
			} else {// 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
				wchar_t& wideChar = out[d++];

				//wideChar = (in[s + 0] & 0x07) << 24;
				//wideChar = (in[s + 1] & 0x3F) << 18;
				wideChar = (in[s + 2] & 0x3F) << 12;
				wideChar |= (in[s + 3] & 0x3F) << 6;
				wideChar |= (in[s + 4] & 0x3F);
				s += 5;
			}
		}

		return d;
	}

	void String::split(const std::string& input, const std::regex& separator, std::vector<std::string>& dst) {
		std::sregex_token_iterator itr(input.begin(), input.end(), separator, -1);
		std::sregex_token_iterator end;
		while (itr != end) dst.emplace_back(*itr++);
	}

	void String::split(const std::string_view& input, const std::string_view& separator, std::vector<std::string_view>& dst) {
		size_t first = 0, size = input.size();
		while (first < size) {
			auto second = input.find_first_of(separator, first);
			if (first != second) dst.emplace_back(input.substr(first, second - first));
			if (second == std::string_view::npos) break;
			first = second + 1;
		}
	}

	std::string String::toString(const ui8* value, ui32 size) {
		std::string str(size << 1, 0);
		i8 buf[3];
		for (ui32 i = 0; i < size; ++i) {
			snprintf(buf, sizeof(buf), "%02x", value[i]);
			ui32 idx = i << 1;
			str[idx++] = buf[0];
			str[idx] = buf[1];
		}
		return std::move(str);
	}

	bool String::isEqual(const i8* str1, const i8* str2) {
		if (str1 == str2) return true;
		
		ui32 i = 0;
		do {
			if (str1[i] != str2[i]) return false;
			if (str1[i] == 0) return true;
			++i;
		} while(true);
	}
}