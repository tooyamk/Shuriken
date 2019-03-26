#pragma once

#include "base/LowLevel.h"
#include <regex>
#include <string>

namespace aurora {
	class AE_DLL String {
	public:
		static void AE_CALL calcUnicodeToUtf8Length(const wchar_t* in, ui32 inLen, ui32& unicodeLen, ui32& utf8Len);
		static i32 AE_CALL UnicodeToUtf8(const wchar_t* in, ui32 inLen, char* out, ui32 outLen);
		static std::string AE_CALL UnicodeToUtf8(const std::wstring& in);
		static void AE_CALL calcUtf8ToUnicodeLength(const i8* in, ui32 inLen, ui32& utf8Len, ui32& unicodeLen);
		static i32 AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out, ui32 outLen);
		static std::wstring AE_CALL Utf8ToUnicode(const std::string& in);
		static i32 AE_CALL Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t*& out);

		static inline std::string AE_CALL toString(i8 value) {
			char buf[5];
			snprintf(buf, sizeof(buf), "%d", value);
			return std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(ui8 value) {
			char buf[4];
			snprintf(buf, sizeof(buf), "%u", value);
			return std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(i16 value) {
			char buf[7];
			snprintf(buf, sizeof(buf), "%d", value);
			return std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(ui16 value) {
			char buf[6];
			snprintf(buf, sizeof(buf), "%u", value);
			return std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(i32 value) {
			char buf[12];
			snprintf(buf, sizeof(buf), "%d", value);
			return std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(ui32 value) {
			char buf[11];
			snprintf(buf, sizeof(buf), "%u", value);
			return std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(const i64& value) {
			char buf[21];
			snprintf(buf, sizeof(buf), "%lld", value);
			return std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(const ui64& value) {
			char buf[21];
			snprintf(buf, sizeof(buf), "%llu", value);
			return std::move(std::string(buf));
		}

		
		static inline std::string AE_CALL toString(float value, const std::string& prec = "", bool trim = true) {
			char buf[16];
			if (prec.empty()) {
				snprintf(buf, sizeof(buf), "%.7f", value);
			} else {
				std::string p = "%." + prec + "f";
				snprintf(buf, sizeof(buf), p.c_str(), value);
			}
			return trim ? _trimFloat(std::string(buf)) : std::move(std::string(buf));
		}

		static inline std::string AE_CALL toString(const double& value, const std::string& prec = "", bool trim = true) {
			char buf[33];
			if (prec.empty()) {
				snprintf(buf, sizeof(buf), "%.15lf", value);
			} else {
				std::string p = "%." + prec + "lf";
				snprintf(buf, sizeof(buf), p.c_str(), value);
			}
			return trim ? _trimFloat(std::string(buf)) : std::move(std::string(buf));
		}

		/*
		static inline std::string AE_CALL toString(const unsigned char* value, unsigned int size) {
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

	private:
		inline static std::regex _trimFloatZero = std::regex("0+?$");
		inline static std::regex _trimFloatPoint = std::regex("[.]$");

		static ui32 AE_CALL _UnicodeToUtf8(const wchar_t * in, ui32 inLen, char* out);
		static ui32 AE_CALL _Utf8ToUnicode(const i8* in, ui32 inLen, wchar_t* out);

		static inline std::string AE_CALL _trimFloat(const std::string& value) {
			if (value.find_first_of('.') > 0) {
				return std::move(std::regex_replace(std::regex_replace(value, _trimFloatZero, ""), _trimFloatPoint, ""));
			} else {
				return value;
			}
		}
	};
}