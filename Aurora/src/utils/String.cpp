#include "String.h"

namespace aurora {
	ui32 String::calcUnicodeToUtf8Length(const wchar_t* in, ui32 inLen) {
		if (in == nullptr) return 0;

		ui32 s = 0, d = 0;
		while (s < inLen) {
			wchar_t c = in[s++];
			if (c < 0x80) {  //
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

		return d;
	}

	i32 String::UnicodeToUtf8(const wchar_t * in, ui32 inLen, char* out, ui32 outLen) {
		if (out == nullptr || in == nullptr || outLen < calcUnicodeToUtf8Length(in, inLen)) return -1;
		return _UnicodeToUtf8(in, inLen, out, outLen);
	}

	std::string String::UnicodeToUtf8(const std::wstring& in) {
		ui32 outLen = calcUnicodeToUtf8Length(in.c_str(), in.size()) + 1;
		i8* out = new i8[outLen];
		auto len = _UnicodeToUtf8(in.c_str(), in.size(), out, outLen);
		out[len] = 0;

		std::string s(out);
		delete[] out;

		return std::move(s);
	}

	ui32 String::calcUtf8ToUnicodeLength(const i8* in, ui32 inLen) {
		if (in == nullptr) return 0;

		ui32 d = 0;
		for (ui32 i = 0; i < inLen; ++i) {
			ui8 c = in[i];
			if ((c & 0x80) == 0) {
				++d;
			} else if ((c & 0xE0) == 0xC0) {// 110x-xxxx 10xx-xxxx
				d += 2;
			} else if ((c & 0xF0) == 0xE0) {// 1110-xxxx 10xx-xxxx 10xx-xxxx
				d += 3;
			} else if ((c & 0xF8) == 0xF0) {// 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
				d += 4;
			} else {// 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
				d += 5;
			}
		}

		return d;
	}

	i32 String::Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen) {
		if (out == nullptr || in == nullptr || outLen < calcUtf8ToUnicodeLength(in, inLen)) return -1;
		return _Utf8ToUnicode(in, inLen, out, outLen);
	}

	std::wstring String::Utf8ToUnicode(const std::string& in) {
		ui32 outLen = calcUtf8ToUnicodeLength(in.c_str(), in.size()) + 1;
		wchar_t* out = new wchar_t[outLen];
		auto len = _Utf8ToUnicode(in.c_str(), in.size(), out, outLen);
		out[len] = 0;

		std::wstring s(out);
		delete[] out;

		return std::move(s);
	}

	ui32 String::_UnicodeToUtf8(const wchar_t* in, ui32 inLen, char* out, ui32 outLen) {
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

	ui32 String::_Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen) {
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
}