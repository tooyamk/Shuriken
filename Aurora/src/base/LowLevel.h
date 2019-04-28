#pragma once

#pragma warning(disable : 4251)
#pragma warning(disable : 4267)
#pragma warning(disable : 4275)
#pragma warning(disable : 4996)

#define AE_OS_PLATFORM_UNKNOWN 0
#define AE_OS_PLATFORM_WIN     1
#define AE_OS_PLATFORM_MAC     2

#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_UNKNOWN

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDLL)
#undef  AE_TARGET_OS_PLATFORM
#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_WIN
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

#ifdef _WIN64
#define AE_X64
#endif


#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC;
#include <unistd.h>
#endif


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
inline constexpr __ENUM__ operator&(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 & (ui32)e2); \
} \
inline constexpr __ENUM__ operator|(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 | (ui32)e2); \
} \
inline constexpr __ENUM__ operator^(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 ^ (ui32)e2); \
} \
inline constexpr __ENUM__ operator~(__ENUM__ e) { \
	return (__ENUM__)(~(ui32)e); \
} \
inline constexpr __ENUM__& operator&=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 & e2; \
	return e1; \
} \
inline constexpr __ENUM__& operator|=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 | e2; \
	return e1; \
} \
inline constexpr __ENUM__& operator^=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 ^ e2; \
	return e1; \
} \


using i8   = char;
using ui8  = unsigned char;
using i16  = short;
using ui16  = unsigned short;
using i32  = int;
using ui32 = unsigned int;
using i64  = long long;
using ui64 = unsigned long long;
using f32  = float;
using f64  = double;

#ifdef AE_X64
using uint_ptr = ui64;
#else
using uint_ptr = ui32;
#endif

namespace aurora {
	static void print(const i8* msg, va_list args) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		const ui32 defaultBufLen = 1024;

		auto size = _vscprintf(msg, args) + 1;
		if (size < defaultBufLen) {
			i8 strBuf[defaultBufLen] = { 0 };
			_vsnprintf_s(strBuf, sizeof(strBuf) - 1, msg, args);

			wchar_t wstrBuf[defaultBufLen] = { 0 };
			MultiByteToWideChar(CP_UTF8, NULL, strBuf, strlen(strBuf), wstrBuf, defaultBufLen);
			OutputDebugStringW(wstrBuf);
		} else {
			auto strBuf = new i8[size];
			_vsnprintf_s(strBuf, size, size, msg, args);

			auto wstrBuf = new wchar_t[size + 1];
			MultiByteToWideChar(CP_UTF8, NULL, strBuf, size, wstrBuf, size + 1);
			OutputDebugStringW(wstrBuf);

			delete[] strBuf;
			delete[] wstrBuf;
		}
#endif
	}

	static void print(const i8* msg, ...) {
		va_list args;
		va_start(args, msg);
		print(msg, args);
		va_end(args);
	}

	static void println(const i8* msg, ...) {
		va_list args;
		va_start(args, msg);
		print(msg, args);
		va_end(args);

		print("\n");
	}
}