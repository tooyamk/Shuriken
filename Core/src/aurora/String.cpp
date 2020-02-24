#include "String.h"

namespace aurora {
	std::tuple<size_t, size_t> String::calcUnicodeToUtf8Length(const std::wstring_view& in) {
		size_t s = 0, d = 0;
		
		auto inSize = in.size();
		while (s < inSize) {
			if (wchar_t c = in[s++]; c == 0) {
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
		
		return std::make_tuple(s, d);
	}

	std::string::size_type String::UnicodeToUtf8(const std::wstring_view& in, char* outBuffer, size_t outBufferSize) {
		if (in.empty() || !outBuffer) return std::string::npos;

		auto [unicodeLen, utf8Len] = calcUnicodeToUtf8Length(in);
		if (outBufferSize < unicodeLen) return std::string::npos;

		return _UnicodeToUtf8(in.data(), unicodeLen, outBuffer);
	}

	void String::calcUtf8ToUnicodeLength(const char* in, size_t inLen, size_t& utf8Len, size_t& unicodeLen) {
		size_t s = 0, d = 0;
		if (in) {
			for (; s < inLen;) {
				if (uint8_t c = in[s]; c == 0) {
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

	std::string::size_type String::Utf8ToUnicode(const char* in, size_t inLen, wchar_t* out, size_t outLen) {
		if (!in || !out) return std::string::npos;

		size_t utf8Len, unicodeLen;
		calcUtf8ToUnicodeLength(in, inLen, utf8Len, unicodeLen);
		if (outLen < unicodeLen) return -1;

		return _Utf8ToUnicode(in, utf8Len, out);
	}

	std::string::size_type String::Utf8ToUnicode(const char* in, size_t inLen, wchar_t*& out) {
		if (!in) return std::string::npos;

		size_t utf8Len, unicodeLen;
		calcUtf8ToUnicodeLength(in, inLen, utf8Len, unicodeLen);
		++unicodeLen;
		out = new wchar_t[unicodeLen];
		auto len = _Utf8ToUnicode(in, utf8Len, out);
		out[len] = 0;

		return len;
	}

	size_t String::_UnicodeToUtf8(const wchar_t* in, size_t inLen, char* out) {
		size_t s = 0, d = 0;
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

	size_t String::_Utf8ToUnicode(const char* in, size_t inLen, wchar_t* out) {
		size_t s = 0, d = 0;
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

	void String::split(const std::string_view& input, const std::regex& separator, std::vector<std::string_view>& dst) {
		std::regex_token_iterator itr(input.begin(), input.end(), separator, -1);
		std::regex_token_iterator<std::string_view::const_iterator> end;
		while (itr != end) {
			dst.emplace_back(&*itr->first, itr->length());
			++itr;
		}
	}

	void String::split(const std::string_view& input, const std::string_view& separator, std::vector<std::string_view>& dst) {
		size_t i = 0, size = input.size(), step = separator.size();
		while (i < size) {
			auto f = input.find_first_of(separator, i);
			if (f == std::string_view::npos) {
				dst.emplace_back(input.data() + i, size - i);
				return;
			} else {
				dst.emplace_back(input.data() + i, f - i);
			}
			
			i = f + step;
		}

		dst.emplace_back(nullptr, 0);
	}

	void String::split(const std::string_view& input, uint8_t flags, std::vector<std::string_view>& dst) {
		size_t begin = 0, i = 0, size = input.size();
		while (i < size) {
			if (CHARS[input[i]] & flags) {
				dst.emplace_back(input.data() + begin, i - begin);
				++i;
				begin = i;
			} else {
				++i;
			}
		}
		dst.emplace_back(input.data() + begin, i - begin);
	}

	std::string_view String::trimQuotation(const std::string_view& str) {
		auto size = str.size();

		if (size >= 2) {
			if (str[0] == '\"' && str[size - 1] == '\"') {
				return std::string_view(str.data() + 1, size - 2);
			} else {
				return str;
			}
		} else if (size == 1 && str[0] == '\"') {
			return std::string_view();
		} else {
			return str;
		}
	}

	std::string_view String::trim(const std::string_view& str, uint8_t flags) {
		auto size = str.size();

		if (size) {
			size_t left = 0, right = size - 1;
			do {
				if (CHARS[str[left]] & flags) {
					++left;
				} else {
					break;
				}
			} while (left < size);

			if (left == size) return std::string_view();

			while (right > left) {
				if (CHARS[str[right]] & flags) {
					--right;
				} else {
					break;
				}
			}

			return std::string_view(str.data() + left, right - left + 1);
		} else {
			return str;
		}
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

	std::string::size_type String::findFirst(const std::string_view& src, const std::string_view& value) {
		if (src.empty() || value.empty()) return std::string::npos;

		auto valSize = value.size();
		for (size_t i = 0, srcSize = src.size(); i < srcSize; ++i) {
			if (src[i] == value[0]) {
				bool equal = true;
				for (size_t j = 1; j < valSize; ++j) {
					if (src[i + j] != value[j]) {
						equal = false;
						break;
					}
				}
				if (equal) return i;
			}
		}
		return std::string::npos;
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