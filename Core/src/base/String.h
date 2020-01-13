#pragma once

#include "base/Global.h"
#include <regex>
#include <string_view>

namespace aurora {
	class AE_DLL String {
	public:
		static void AE_CALL calcUnicodeToUtf8Length(const wchar_t* in, size_t inLen, size_t& unicodeLen, size_t& utf8Len);
		static std::string::size_type AE_CALL UnicodeToUtf8(const wchar_t* in, size_t inLen, char* out, size_t outLen);
		
		template<typename T,
		typename = typename std::enable_if_t<is_wstring_v<T>, T>>
		static std::string AE_CALL UnicodeToUtf8(const T& in) {
			size_t unicodeLen, utf8Len;
			calcUnicodeToUtf8Length(in.data(), in.size(), unicodeLen, utf8Len);
			std::string s;
			s.resize(utf8Len);
			_UnicodeToUtf8(in.data(), unicodeLen, (char*)s.data());

			return std::move(s);
		}

		inline static std::string AE_CALL UnicodeToUtf8(const wchar_t* in) {
			return std::move(UnicodeToUtf8(std::wstring_view(in, wcslen(in))));
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

		static std::string_view trimQuotation(const std::string_view& str);
		static std::string_view trimSpace(const std::string_view& str);

		template<typename T,
		typename = typename std::enable_if_t<std::is_integral_v<T>, T>>
		inline static std::string AE_CALL toString(T value, uint8_t base = 10) {
			char buf[21];
			auto rst = std::to_chars(buf, buf + sizeof(buf), value, base);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
		}

		template<typename T,
		typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
		inline static std::string AE_CALL toString(T value, std::chars_format fmt = std::chars_format::general) {
			char buf[33];
			auto rst = std::to_chars(buf, buf + sizeof(buf), value, fmt);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
		}

		static std::string AE_CALL toString(const uint8_t* value, size_t size);

		template<typename T,
		typename = typename std::enable_if_t<std::is_integral_v<T>, T>>
		inline static T toNumber(const char* in, size_t size, const int32_t base = 10) {
			T value;
			return std::from_chars(in, in + size, value, base).ec == std::errc() ? value : 0;
		}

		template<typename T,
		typename = typename std::enable_if_t<std::is_integral_v<T>, T>>
		inline static T toNumber(const std::string_view& in, const int32_t base = 10) {
			return toNumber<T>(in.data(), in.size(), base);
		}

		template<typename T,
		typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
		inline static T toNumber(const char* in, size_t size, std::chars_format fmt = std::chars_format::general) {
			T value;
			return std::from_chars(in, in + size, value, fmt).ec == std::errc() ? value : 0.;
		}

		template<typename T,
		typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
		inline static T toNumber(const std::string_view& in, std::chars_format fmt = std::chars_format::general) {
			return toNumber<T>(in.data(), in.size(), fmt);
		}

		inline static std::string::size_type AE_CALL findFirst(const char* src, size_t srcSize, char c) {
			for (size_t i = 0; i < srcSize; ++i) {
				if (src[i] == c) return i;
			}
			return std::string::npos;
		}

		static std::string::size_type AE_CALL findFirst(const char* src, size_t srcSize, const char* value, size_t valueSize);

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