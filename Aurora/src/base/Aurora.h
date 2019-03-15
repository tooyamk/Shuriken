#pragma once

#include <chrono>

#define AE_NS       aurora
#define AE_NS_BEGIN namespace AE_NS {
#define AE_NS_END   };
#define AE_FULL_NS  AE_NS
#define USING_AE_NS using namespace AE_FULL_NS;

#define AE_MODULE_NS       AE_NS::module
#define AE_MODULE_NS_BEGIN AE_NS_BEGIN namespace module {
#define AE_MODULE_NS_END   }; AE_NS_END
#define USING_AE_MODULE_NS using namespace AE_MODULE_NS;

#define AE_MODULE_GRAPHICS_NS       AE_MODULE_NS::graphics
#define AE_MODULE_GRAPHICS_NS_BEGIN AE_MODULE_NS_BEGIN namespace graphics {
#define AE_MODULE_GRAPHICS_NS_END   }; AE_MODULE_NS_END
#define USING_AE_MODULE_GRAPHICS_NS using namespace AE_MODULE_GRAPHICS_NS;

#define AE_NODE_NS       AE_NS::node
#define AE_NODE_NS_BEGIN AE_NS_BEGIN namespace node {
#define AE_NODE_NS_END   }; AE_NS_END
#define USING_AE_NODE_NS using namespace AE_NODE_NS;

#define AE_NODE_COMPONENT_NS       AE_NODE_NS::component
#define AE_NODE_COMPONENT_NS_BEGIN AE_NODE_NS_BEGIN namespace component {
#define AE_NODE_COMPONENT_NS_END   }; AE_NODE_NS_END
#define USING_AE_NODE_COMPONENT_NS using namespace AE_NODE_COMPONENT_NS;

#define AE_EVENT_NS       AE_NS::event
#define AE_EVENT_NS_BEGIN AE_NS_BEGIN namespace event {
#define AE_EVENT_NS_END   }; AE_NS_END
#define USING_AE_EVENT_NS using namespace AE_EVENT_NS;


#define AE_OS_PLATFORM_UNKNOWN 0
#define AE_OS_PLATFORM_WIN32   1
#define AE_OS_PLATFORM_MAC     2

#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_UNKNOWN

#if defined(_WIN32) || defined(_WINDLL)
#undef  AE_TARGET_OS_PLATFORM
#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_WIN32
#endif


#define AE_CALL __fastcall


#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "atlbase.h"
#include "atlstr.h"

#ifdef _MSC_VER
#define sleep(ms) Sleep(ms * 1000)
#endif

#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC;
#include <unistd.h>
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


typedef char	           i8;
typedef unsigned char      ui8;
typedef short              i16;
typedef unsigned short     ui16;
typedef int                i32;
typedef unsigned int       ui32;
typedef long long          i64;
typedef unsigned long long ui64;
typedef float              f32;
typedef double             f64;

namespace aurora {}

AE_NS_BEGIN

template<typename T>
inline constexpr ui32 AE_CALL nsizeof(ui32 n) {
	return n * sizeof(T);
}

template<typename T>
inline constexpr ui32 AE_CALL nsizeof(T v, ui32 n) {
	return n * sizeof(v);
}

template<typename Time, typename Clock>
inline static i64 AE_CALL getTimeNow() {
	return std::chrono::time_point_cast<Time>(Clock::now()).time_since_epoch().count();
}

static void print(const i8* msg, va_list args) {
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN32
	i8 strBuffer[4096] = { 0 };
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, msg, args);
	OutputDebugString(CA2W(strBuffer));
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

AE_NS_END