#pragma once

#include "base/LowLevel.h"
#include <regex>

namespace aurora {
	class AE_DLL String {
	public:
		static void AE_CALL calcUnicodeToUtf8Length(const wchar_t* in, ui32 inLen, ui32& unicodeLen, ui32& utf8Len);
		static std::string::size_type AE_CALL UnicodeToUtf8(const wchar_t* in, ui32 inLen, i8* out, ui32 outLen);
		static std::string AE_CALL UnicodeToUtf8(const std::wstring& in);
		static void AE_CALL calcUtf8ToUnicodeLength(const i8* in, ui32 inLen, ui32& utf8Len, ui32& unicodeLen);
		static std::string::size_type AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen);
		static std::wstring AE_CALL Utf8ToUnicode(const std::string& in);
		static std::string::size_type AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t*& out);

		inline static void AE_CALL split(const std::string& input, const std::string& separator, std::vector<std::string>& dst) {
			split(input, std::regex("\\" + separator), dst);
		}
		static void AE_CALL split(const std::string& input, const std::regex& separator, std::vector<std::string>& dst);
		static void AE_CALL split(const std::string_view& input, const std::string_view& separator, std::vector<std::string_view>& dst);

		template<typename T,
		typename = typename std::enable_if_t<std::is_integral_v<T>, T>>
		inline static std::string AE_CALL toString(T value) {
			i8 buf[21];
			auto rst = std::to_chars(buf, buf + sizeof(buf), value);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
		}

		template<typename T,
		typename = typename std::enable_if_t<std::is_floating_point_v<T>, T>>
		inline static std::string AE_CALL toString(T value, std::chars_format fmt = std::chars_format::general) {
			i8 buf[33];
			auto rst = std::to_chars(buf, buf + sizeof(buf), value, fmt);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
		}

		static std::string AE_CALL toString(const ui8* value, ui32 size);

		inline static std::string::size_type AE_CALL findFirst(const i8* src, ui32 srcSize, i8 c) {
			for (ui32 i = 0; i < srcSize; ++i) {
				if (src[i] == c) return i;
			}
			return std::string::npos;
		}

		static std::string::size_type AE_CALL findFirst(const i8* src, ui32 srcSize, const i8* value, ui32 valueSize);

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

		static bool AE_CALL isEqual(const i8* str1, const i8* str2);

	private:
		static ui32 AE_CALL _UnicodeToUtf8(const wchar_t* in, ui32 inLen, i8* out);
		static ui32 AE_CALL _Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out);
	};
}