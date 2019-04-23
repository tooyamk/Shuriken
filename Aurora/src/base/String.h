#pragma once

#include "base/LowLevel.h"
#include <regex>
#include <string>

namespace aurora {
	class AE_DLL String {
	public:
		static void AE_CALL calcUnicodeToUtf8Length(const wchar_t* in, ui32 inLen, ui32& unicodeLen, ui32& utf8Len);
		static i32 AE_CALL UnicodeToUtf8(const wchar_t* in, ui32 inLen, i8* out, ui32 outLen);
		static std::string AE_CALL UnicodeToUtf8(const std::wstring& in);
		static void AE_CALL calcUtf8ToUnicodeLength(const i8* in, ui32 inLen, ui32& utf8Len, ui32& unicodeLen);
		static i32 AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen);
		static std::wstring AE_CALL Utf8ToUnicode(const std::string& in);
		static i32 AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t*& out);

		inline static void AE_CALL split(const std::string& input, const std::string& par, std::vector<std::string>& dst) {
			split(input, std::regex("\\" + par), dst);
		}
		static void AE_CALL split(const std::string& input, const std::regex& par, std::vector<std::string>& dst);

		inline static std::string AE_CALL toString(i8 value) {
			i8 buf[5];
			snprintf(buf, sizeof(buf), "%d", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(ui8 value) {
			i8 buf[4];
			snprintf(buf, sizeof(buf), "%u", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(i16 value) {
			i8 buf[7];
			snprintf(buf, sizeof(buf), "%d", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(ui16 value) {
			i8 buf[6];
			snprintf(buf, sizeof(buf), "%u", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(i32 value) {
			i8 buf[12];
			snprintf(buf, sizeof(buf), "%d", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(ui32 value) {
			i8 buf[11];
			snprintf(buf, sizeof(buf), "%u", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(const i64& value) {
			i8 buf[21];
			snprintf(buf, sizeof(buf), "%lld", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(const ui64& value) {
			i8 buf[21];
			snprintf(buf, sizeof(buf), "%llu", value);
			return std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(f32 value, const std::string& prec = "", bool trim = true) {
			i8 buf[16];
			if (prec.empty()) {
				snprintf(buf, sizeof(buf), "%.7f", value);
			} else {
				std::string p = "%." + prec + "f";
				snprintf(buf, sizeof(buf), p.c_str(), value);
			}
			return trim ? _trimFloat(std::string(buf)) : std::move(std::string(buf));
		}

		inline static std::string AE_CALL toString(const f64& value, const std::string& prec = "", bool trim = true) {
			i8 buf[33];
			if (prec.empty()) {
				snprintf(buf, sizeof(buf), "%.15lf", value);
			} else {
				std::string p = "%." + prec + "lf";
				snprintf(buf, sizeof(buf), p.c_str(), value);
			}
			return trim ? _trimFloat(std::string(buf)) : std::move(std::string(buf));
		}

		static std::string AE_CALL toString(const ui8* value, ui32 size);

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
		static ui32 AE_CALL _UnicodeToUtf8(const wchar_t * in, ui32 inLen, i8* out);
		static ui32 AE_CALL _Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out);

		inline static std::string AE_CALL _trimFloat(const std::string& value) {
			if (value.find_first_of('.') > 0) {
				return std::move(std::regex_replace(std::regex_replace(value, std::regex("0+?$"), ""), std::regex("[.]$"), ""));
			} else {
				return value;
			}
		}
	};
}