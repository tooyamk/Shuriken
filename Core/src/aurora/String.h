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

		//unicodeLen, utf8Len
		static std::tuple<size_t, size_t> AE_CALL calcUnicodeToUtf8Length(const std::wstring_view& in);
		static std::string::size_type AE_CALL UnicodeToUtf8(const std::wstring_view& in, char* outBuffer, size_t outBufferSize);
		
		template<typename T,
		typename = typename std::enable_if_t<is_wstring_v<T>, T>>
		static std::string AE_CALL UnicodeToUtf8(const T& in) {
			auto [unicodeLen, utf8Len] = calcUnicodeToUtf8Length(in);
			std::string s;
			s.resize(utf8Len);
			_UnicodeToUtf8(in.data(), unicodeLen, (char*)s.data());

			return std::move(s);
		}

		static void AE_CALL calcUtf8ToUnicodeLength(const char* in, size_t inLen, size_t& utf8Len, size_t& unicodeLen);
		static std::string::size_type AE_CALL Utf8ToUnicode(const char* in, size_t inLen, wchar_t* out, size_t outLen);
		
		template<typename T,
		typename = typename std::enable_if_t<is_string_v<T>, T>>
		static std::wstring AE_CALL Utf8ToUnicode(const T& in) {
			size_t utf8Len, unicodeLen;
			calcUtf8ToUnicodeLength(in.data(), in.size(), utf8Len, unicodeLen);
			std::wstring s;
			s.resize(unicodeLen);
			_Utf8ToUnicode(in.data(), utf8Len, (wchar_t*)s.data());

			return std::move(s);
		}

		inline static std::wstring AE_CALL Utf8ToUnicode(const char* in) {
			return std::move(Utf8ToUnicode(std::string_view(in, strlen(in))));
		}

		static std::string::size_type AE_CALL Utf8ToUnicode(const char* in, size_t inLen, wchar_t*& out);

		//inline static void AE_CALL split(const std::string_view& input, const std::string& separator, std::vector<std::string_view>& dst) {
		//	split(input, std::regex("\\" + separator), dst);
		//}
		static void AE_CALL split(const std::string_view& input, const std::regex& separator, std::vector<std::string_view>& dst);
		static void AE_CALL split(const std::string_view& input, const std::string_view& separator, std::vector<std::string_view>& dst);
		static void AE_CALL split(const std::string_view& input, uint8_t flags, std::vector<std::string_view>& dst);

		static std::string_view trimQuotation(const std::string_view& str);
		static std::string_view trim(const std::string_view& str, uint8_t flags);

		template<typename T, typename = typename std::enable_if_t<std::is_integral_v<T>, T>>
		inline static std::string AE_CALL toString(T value, uint8_t base = 10) {
			char buf[21];
#if __cpp_lib_to_chars
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

		template<typename T, typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
		inline static std::string AE_CALL toString(T value) {
			char buf[33];
#if __cpp_lib_to_chars
			auto rst = std::to_chars(buf, buf + sizeof(buf), value);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
#else
			if constexpr (std::is_same_v<T, f32>) {
				snprintf(buf, sizeof(buf), "%f", value);
			} else if constexpr (std::is_same_v<T, f64>) {
				snprintf(buf, sizeof(buf), "%lf", value);
			}

			return std::move(std::string(buf));
#endif
		}

		static std::string AE_CALL toString(const uint8_t* value, size_t size);

		template<typename T, typename = typename std::enable_if_t<std::is_integral_v<T>, T>>
		inline static T toNumber(const std::string_view& in, int32_t base = 10) {
			T value;
#if __cpp_lib_to_chars
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

		template<typename T, typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
		inline static T toNumber(const std::string_view& in, size_t size) {
			T value;
#if __cpp_lib_to_chars
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

		inline static std::string::size_type AE_CALL findFirst(const std::string_view& src, char c) {
			for (size_t i = 0, n = src.size(); i < n; ++i) {
				if (src[i] == c) return i;
			}
			return std::string::npos;
		}

		static std::string::size_type AE_CALL findFirst(const std::string_view& src, const std::string_view& value);

		inline static std::string::size_type AE_CALL findFirst(const std::string_view input, uint8_t flags) {
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
		static size_t AE_CALL _UnicodeToUtf8(const wchar_t* in, size_t inLen, char* out);
		static size_t AE_CALL _Utf8ToUnicode(const char* in, size_t inLen, wchar_t* out);
	};
}