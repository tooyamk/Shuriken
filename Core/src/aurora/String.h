#pragma once

#include "aurora/Global.h"
#include <regex>
#include <string_view>

namespace aurora {
	class AE_DLL String {
	public:
		struct AE_DLL CharFlag {
			inline static constexpr uint8_t WHITE_SPACE = 0b1;
			inline static constexpr uint8_t NEW_LINE = 0b10;
		};

		
		inline static constexpr uint8_t CHARS[] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, CharFlag::WHITE_SPACE,  //  0-9 9=\t
			CharFlag::WHITE_SPACE | CharFlag::NEW_LINE, CharFlag::WHITE_SPACE, CharFlag::WHITE_SPACE, CharFlag::WHITE_SPACE | CharFlag::NEW_LINE, 0, 0, 0, 0, 0, 0,  // 10- 19 10=\n, 11=\v, 12=\f, 13=\r
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 20- 29
			0, 0, CharFlag::WHITE_SPACE, 0, 0, 0, 0, 0, 0, 0,  // 30- 39 32=space
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 40- 49
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 50- 59
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 60- 69
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 70- 79
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 80- 89
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 90- 99
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //100-109
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //110-119
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //120-129
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //130-139
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //140-149
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //150-159
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //160-169
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //170-179
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //180-189
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //190-199
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //200-209
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //210-219
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //220-229
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //230-239
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //240-249
			0, 0, 0, 0, 0, 0			   //250-255
		};

		static bool AE_CALL isUTF8(const char* data, size_t len);

		//unicodeLen, utf8Len
		template<typename T, typename = wstring_data_t<T>>
		static std::tuple<std::wstring::size_type, std::string::size_type> AE_CALL calcUnicodeToUtf8Length(const T& in) {
			std::wstring::size_type s = 0;
			std::string::size_type d = 0;

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

		template<typename T, typename = wstring_data_t<T>>
		static std::string::size_type AE_CALL UnicodeToUtf8(const T& in, char* outBuffer, std::string::size_type outBufferSize) {
			if (!outBuffer || !outBufferSize) return std::string::npos;
			if (in.empty()) {
				outBuffer[0] = 0;
				return 0;
			}

			auto [unicodeLen, utf8Len] = calcUnicodeToUtf8Length(in);
			if (outBufferSize < unicodeLen) return std::string::npos;

			return _UnicodeToUtf8(in.data(), unicodeLen, outBuffer);
		}
		
		template<typename T, typename = wstring_data_t<T>>
		static std::string AE_CALL UnicodeToUtf8(const T& in) {
			auto [unicodeLen, utf8Len] = calcUnicodeToUtf8Length(in);
			std::string s;
			s.resize(utf8Len);
			_UnicodeToUtf8(in.data(), unicodeLen, (char*)s.data());

			return std::move(s);
		}

		//utf8Len, unicodeLen
		template<typename T, typename = string_data_t<T>>
		static std::tuple<std::string::size_type, std::wstring::size_type> AE_CALL calcUtf8ToUnicodeLength(const T& in) {
			std::string::size_type s = 0;
			std::wstring::size_type d = 0;

			auto inSize = in.size();
			while (s < inSize) {
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

			return std::make_tuple(s, d);
		}

		template<typename T, typename = string_data_t<T>>
		static std::wstring::size_type AE_CALL Utf8ToUnicode(const T& in, wchar_t* outBuffer, std::wstring::size_type outBufferSize) {
			if (!outBuffer || !outBufferSize) return std::wstring::npos;
			if (in.empty()) {
				outBuffer[0] = 0;
				return 0;
			}

			auto [utf8Len, unicodeLen] = calcUtf8ToUnicodeLength(in);
			if (outBufferSize < unicodeLen) return std::wstring::npos;

			return _Utf8ToUnicode(in, utf8Len, outBuffer);
		}
		
		template<typename T, typename = string_data_t<T>>
		static std::wstring AE_CALL Utf8ToUnicode(const T& in) {
			auto [utf8Len, unicodeLen] = calcUtf8ToUnicodeLength(in);
			std::wstring s;
			s.resize(unicodeLen);
			_Utf8ToUnicode(in.data(), utf8Len, s.data());

			return std::move(s);
		}

		template<typename T, typename = string_data_t<T>>
		static std::wstring::size_type AE_CALL Utf8ToUnicode(const T& in, wchar_t*& out) {
			if (in.empty()) {
				out = new wchar_t[1];
				out[0] = 0;
				return 0;
			}

			auto [utf8Len, unicodeLen] = calcUtf8ToUnicodeLength(in);
			++unicodeLen;
			out = new wchar_t[unicodeLen];
			auto len = _Utf8ToUnicode(in.data(), utf8Len, out);
			out[len] = 0;

			return len;
		}

		template<typename Input, typename Separator, typename Fn, typename = string_data_t<Input>, typename = std::enable_if_t<(std::is_base_of_v<std::regex, Separator> || is_string_data_v<Separator> || std::is_convertible_v<Separator, char const*>) && std::is_invocable_v<Fn, const std::string&>, Separator>>
		static void AE_CALL split(const Input& input, const Separator& separator, const Fn& fn) {
			if constexpr (std::is_base_of_v<std::regex, Separator>) {
				std::regex_token_iterator itr(input.begin(), input.end(), separator, -1);
				std::regex_token_iterator<Input::const_iterator> end;
				while (itr != end) {
					fn(std::string_view(&*itr->first, itr->length()));
					++itr;
				}
			} else if constexpr (is_string_data_v<Separator>) {
				if (auto step = separator.size(); step) {
					size_t begin = 0, i = 0, size = input.size();
					while (i < size) {
						if (input[i] == separator[0]) {
							auto found = true;
							for (size_t j = 1; j < step; ++j) {
								if (input[i + j] != separator[j]) {
									found = false;
									break;
								}
							}

							if (found) {
								fn(std::string_view(input.data() + begin, i - begin));
								i += step;
								begin = i;
							} else {
								++i;
							}
						} else {
							++i;
						}
					}

					fn(std::string_view(input.data() + begin, i - begin));
				} else {
					fn(input);
				}
			} else {
				split(input, std::string_view(separator), fn);
			}
		}

		template<typename Input, typename Fn, typename = string_data_t<Input>, typename = std::enable_if_t<std::is_invocable_v<Fn, const std::string&>, Fn>>
		static void AE_CALL split(const Input& input, uint8_t flags, const Fn& fn) {
			size_t begin = 0, i = 0, size = input.size();
			while (i < size) {
				if (CHARS[input[i]] & flags) {
					fn(std::string_view(input.data() + begin, i - begin));
					++i;
					begin = i;
				} else {
					++i;
				}
			}

			fn(std::string_view(input.data() + begin, i - begin));
		}

		template<typename T, typename = string_data_t<T>>
		static std::string_view trimQuotation(const T& str) {
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

		template<typename T, typename = string_data_t<T>>
		static std::string_view trim(const T& str, uint8_t flags) {
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

		template<typename T, typename = integral_t<T>>
		inline static std::string AE_CALL toString(T value, uint8_t base = 10) {
			char buf[21];
#ifdef __cpp_lib_to_chars
			auto rst = std::to_chars(buf, buf + sizeof(buf), value, base);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
#else
			if constexpr (std::is_same_v<T, int8_t>) {
				snprintf(buf, sizeof(buf), "%hhd", value);
			} else if constexpr (std::is_same_v<T, uint8_t>) {
				switch(base) {
				case 8:
					snprintf(buf, sizeof(buf), "%hho", value);
				case 16:
					snprintf(buf, sizeof(buf), "%hhx", value);
				default:
					snprintf(buf, sizeof(buf), "%hhu", value);
				}
			} else if constexpr (std::is_same_v<T, int16_t>) {
				snprintf(buf, sizeof(buf), "%hd", value);
			} else if constexpr (std::is_same_v<T, uint16_t>) {
				switch (base) {
				case 8:
					snprintf(buf, sizeof(buf), "%ho", value);
				case 16:
					snprintf(buf, sizeof(buf), "%hx", value);
				default:
					snprintf(buf, sizeof(buf), "%hu", value);
				}
			} else if constexpr (std::is_same_v<T, int32_t>) {
				snprintf(buf, sizeof(buf), "%d", value);
			} else if constexpr (std::is_same_v<T, uint32_t>) {
				switch (base) {
				case 8:
					snprintf(buf, sizeof(buf), "%o", value);
				case 16:
					snprintf(buf, sizeof(buf), "%x", value);
				default:
					snprintf(buf, sizeof(buf), "%u", value);
				}
			} else if constexpr (std::is_same_v<T, int64_t>) {
				snprintf(buf, sizeof(buf), "%lld", value);
			} else if constexpr (std::is_same_v<T, uint64_t>) {
				switch (base) {
				case 8:
					snprintf(buf, sizeof(buf), "%llo", value);
				case 16:
					snprintf(buf, sizeof(buf), "%llx", value);
				default:
					snprintf(buf, sizeof(buf), "%llu", value);
				}
			}

			return std::move(std::string(buf));
#endif
		}

		template<typename T, typename = floating_point_t<T>>
		inline static std::string AE_CALL toString(T value) {
			char buf[33];
#ifdef __cpp_lib_to_chars
			auto rst = std::to_chars(buf, buf + sizeof(buf), value);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
#else
			if constexpr (std::is_same_v<T, float32_t>) {
				snprintf(buf, sizeof(buf), "%f", value);
			} else if constexpr (std::is_same_v<T, float64_t>) {
				snprintf(buf, sizeof(buf), "%lf", value);
			}

			return std::move(std::string(buf));
#endif
		}

		static std::string AE_CALL toString(const uint8_t* value, size_t size);

		template<typename Out, typename In, typename = string_data_t<In>, typename = integral_t<Out>>
		inline static Out toNumber(const In& in, int32_t base = 10) {
			Out value;
#ifdef __cpp_lib_to_chars
			auto begin = in.data();
			return std::from_chars(begin, begin + in.size(), value, base).ec == std::errc() ? value : 0;
#else
			if constexpr (std::is_unsigned_v<T>) {
				if constexpr (sizeof(T) <= 4) {
					return std::stoul(in.data(), nullptr, base);
				} else {
					return std::stoull(in.data(), nullptr, base);
				}
			} else {
				if constexpr (sizeof(T) <= 4) {
					return std::stol(in.data(), nullptr, base);
				} else {
					return std::stoll(in.data(), nullptr, base);
				}
			}
#endif
		}

		template<typename Out, typename In, typename = string_data_t<In>, typename = floating_point_t<Out>>
		inline static Out toNumber(const In& in) {
			Out value;
#ifdef __cpp_lib_to_chars
			auto begin = in.data();
			return std::from_chars(begin, begin + in.size(), value).ec == std::errc() ? value : 0.;
#else
			if constexpr (sizeof(T) <= 4) {
				return std::stof(in.data(), nullptr);
			} else {
				return std::stod(in.data(), nullptr);
			}
#endif
		}

		template<typename T, typename = string_data_t<T>>
		inline static std::string::size_type AE_CALL find(const T& input, char c) {
			auto p = (const char*)memchr(input.data(), c, input.size());
			return p ? p - input.data() : std::string::npos;
		}

		template<typename Input, typename Compare, typename = string_data_t<Input>, typename = std::enable_if_t<is_string_data_v<Compare> || std::is_convertible_v<Compare, char const*>, Compare>>
		inline static std::string::size_type AE_CALL find(const Input& input, const Compare& compare) {
			if constexpr (is_string_data_v<Compare>) {
				auto p = (const char*)memFind(input.data(), input.size(), compare.data(), compare.size());
				return p ? p - input.data() : std::string::npos;
			} else {
				return find(input, std::string_view(compare));
			}
		}

		template<typename T, typename = string_data_t<T>>
		inline static std::string::size_type AE_CALL find(const T& input, uint8_t flags) {
			for (size_t i = 0, n = input.size(); i < n; ++i) {
				if (CHARS[input[i]] & flags) return i;
			}
			return std::string::npos;
		}

		/*
		inline static std::string AE_CALL toString(const unsigned char* value, unsigned int size) {
			std::string str(size << 1, 0);
			char buf[3];
			for (unsigned int i = 0; i < size; ++i) {
				snprintf(buf, sizeof(buf), "%02x", value[i]);
				unsigned int idx = i << 1;
				str[idx++] = buf[0];
				str[idx] = buf[1];
			}
			return std::move(str);
		}
		*/

		static bool AE_CALL isEqual(const char* str1, const char* str2);

	private:
		static std::string::size_type AE_CALL _UnicodeToUtf8(const wchar_t* in, std::wstring::size_type inLen, char* out);
		static std::wstring::size_type AE_CALL _Utf8ToUnicode(const char* in, std::string::size_type inLen, wchar_t* out);
	};
}