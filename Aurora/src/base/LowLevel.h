#pragma once

#pragma warning(disable : 4244)
#pragma warning(disable : 4251)
#pragma warning(disable : 4267)
#pragma warning(disable : 4275)
#pragma warning(disable : 4838)
#pragma warning(disable : 4996)

#define AE_OS_PLATFORM_UNKNOWN 0
#define AE_OS_PLATFORM_WIN     1
#define AE_OS_PLATFORM_MAC     2

#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_UNKNOWN

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDLL)
#undef  AE_TARGET_OS_PLATFORM
#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_WIN
#endif


#if defined(_M_X64) || defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(__x86_64)
#define AE_64BITS
#endif


#define AE_CALL __fastcall


#define ae_internal_public public


#define  AE_CREATE_MODULE_FN_NAME ae_create_module


#define _AE_TO_STRING(str) #str
#define AE_TO_STRING(str) _AE_TO_STRING(str)


#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "atlbase.h"
#include "atlstr.h"

#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC;
#include <unistd.h>
#endif


#include <charconv>
#include <string>


#if defined(DEBUG) || defined(_DEBUG)
#define AE_DEBUG
#endif


#ifdef __GNUC__
#define AE_DLL_EXPORT __attribute__((dllexport))
#define AE_DLL_IMPORT __attribute__((dllimport))
#else
#define AE_DLL_EXPORT __declspec(dllexport)
#define AE_DLL_IMPORT __declspec(dllimport)
#endif

#define AE_MODULE_DLL_EXPORT AE_DLL_EXPORT
#define AE_MODULE_DLL_IMPORT AE_DLL_IMPORT

#define AE_TEMPLATE_DLL_EXPORT AE_DLL_EXPORT
#define AE_TEMPLATE_DLL_IMPORT

#define AE_MODULE_DLL_EXPORT AE_DLL_EXPORT
#define AE_MODULE_DLL_IMPORT AE_DLL_IMPORT

#define AE_EXTENSION_DLL_EXPORT AE_DLL_EXPORT
#define AE_EXTENSION_DLL_IMPORT AE_DLL_IMPORT

#ifdef AE_EXPORTS
#define AE_DLL AE_DLL_EXPORT
#define AE_TEMPLATE_DLL AE_TEMPLATE_DLL_EXPORT
#else
#define AE_DLL AE_DLL_IMPORT
#define AE_TEMPLATE_DLL AE_TEMPLATE_DLL_IMPORT
#endif

#ifdef AE_MODULE_EXPORTS
#define AE_MODULE_DLL AE_MODULE_DLL_EXPORT
#else
#define AE_MODULE_DLL AE_MODULE_DLL_IMPORT
#endif

#ifdef AE_EXTENSION_EXPORTS
#define AE_EXTENSION_DLL AE_EXTENSION_DLL_EXPORT
#else
#define AE_EXTENSION_DLL AE_EXTENSION_DLL_IMPORT
#endif


#define AE_DECLA_CANNOT_INSTANTIATE(__CLASS__) \
__CLASS__() = delete; \
__CLASS__(const __CLASS__&) = delete; \
__CLASS__(__CLASS__&&) = delete; \


#define AE_DEFINE_ENUM_BIT_OPERATIION(__ENUM__) \
inline constexpr __ENUM__ AE_CALL operator&(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 & (ui32)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator|(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 | (ui32)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator^(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 ^ (ui32)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator~(__ENUM__ e) { \
	return (__ENUM__)(~(ui32)e); \
} \
inline constexpr __ENUM__& AE_CALL operator&=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 & e2; \
	return e1; \
} \
inline constexpr __ENUM__& AE_CALL operator|=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 | e2; \
	return e1; \
} \
inline constexpr __ENUM__& AE_CALL operator^=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 ^ e2; \
	return e1; \
} \


namespace aurora {
	using i8 = char;
	using ui8 = unsigned char;
	using i16 = short;
	using ui16 = unsigned short;
	using i32 = int;
	using ui32 = unsigned int;
	using i64 = long long;
	using ui64 = unsigned long long;
	using f32 = float;
	using f64 = double;

#ifdef AE_64BITS
	using uint_ptr = ui64;
#else
	using uint_ptr = ui32;
#endif


	template <typename From, typename To, typename... R>
	struct are_all_convertible : std::bool_constant<std::is_convertible_v<From, To> && are_all_convertible<To, R...>::value> {
	};

	template <typename From, typename To>
	struct are_all_convertible<From, To> : std::bool_constant<std::is_convertible_v<From, To>> {
	};

	template<class From, class To, typename... R> inline constexpr bool are_all_convertible_v = are_all_convertible<From, To, R...>::value;


	class AE_TEMPLATE_DLL Console {
	public:
		template<typename... Args>
		inline static void AE_CALL print(Args... args) {
			((_print(args)), ...);
		}

	private:
		template<typename T>
		inline static void AE_CALL _print(const T& value) {
			if constexpr (std::is_convertible_v<T, i8 const*>) {
				_platformPrint(value, strlen(value));
			} else if constexpr (std::is_convertible_v<T, wchar_t const*>) {
				_platformPrint(value);
			} else if constexpr (std::is_same_v<T, std::string>) {
				_platformPrint(value.c_str(), value.size());
			} else if constexpr (std::is_same_v<T, std::wstring>) {
				_platformPrint(value.c_str());
			} else if constexpr (std::is_same_v<T, bool>) {
				_platformPrint(value ? L"true" : L"false");
			} else if constexpr (std::is_arithmetic_v<T>) {
				if constexpr (std::is_integral_v<T>) {
					i8 buf[21];
					auto rst = std::to_chars(buf, buf + sizeof(buf), value);
					if (rst.ec == std::errc()) _platformPrint(buf, rst.ptr - buf);
				} else {
					i8 buf[33];
					auto rst = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general);
					if (rst.ec == std::errc()) _platformPrint(buf, rst.ptr - buf);
				}
			} else if constexpr (std::is_pointer_v<T>) {
				_print((const uint_ptr)value);
			} else {
				_platformPrint(L"[");

				auto str = typeid(T).name();
				_platformPrint(str, strlen(str));

				_platformPrint(L"]");
			}
		}

		static void AE_CALL _platformPrint(const i8* buf, ui32 size) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
			static auto output = [](const i8* in, ui32 inSize, wchar_t* out, ui32 outSize) {
				auto len = MultiByteToWideChar(CP_UTF8, NULL, in, inSize, out, outSize);
				if (len > 0) {
					out[len] = 0;
					OutputDebugStringW(out);
				}
			};

			const ui32 MAX_LEN = 256;
			if (size < MAX_LEN) {
				wchar_t wbuf[MAX_LEN];
				output(buf, size, wbuf, sizeof(wbuf));
			} else {
				wchar_t* wbuf = new wchar_t[size + 1];
				output(buf, size, wbuf, size);
				delete[] wbuf;
			}
#endif
		}

		static void AE_CALL _platformPrint(const wchar_t* buf) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
			OutputDebugStringW(buf);
#endif
		}
	};

	template<typename... Args>
	inline void AE_CALL print(Args... args) {
		Console::print(args...);
	}

	template<typename... Args>
	inline void AE_CALL println(Args... args) {
		Console::print(args...);
		Console::print(L"\n");
	}
}