#pragma once

#include "srk/Std.h"

#if SRK_CPP_VER < SRK_CPP_VER_20
#	error compile srk library need c++20
#endif


#define srk_internal_public public


#define _SRK_TO_STRING(str) #str
#define SRK_TO_STRING(str) _SRK_TO_STRING(str)


#ifdef SRK_CORE_EXPORTS
#	define SRK_CORE_DLL SRK_DLL_EXPORT
#else
#	define SRK_CORE_DLL SRK_DLL_IMPORT
#endif


#ifndef __cpp_lib_destroying_delete
#	error srk library need Destroying operator delete feature
#endif
#ifndef __cpp_concepts
#	error srk library need Concepts feature
#endif

namespace srk {
	class SRK_CORE_DLL Environment {
	public:
		Environment() = delete;

		static constexpr bool IS_DEBUG =
#ifdef SRK_DEBUG
			true;
#else
			false;
#endif

		enum class Compiler : uint8_t {
			UNKNOWN,
			CLANG,
			GCC,
			MSVC
		};

		static constexpr Compiler COMPILER =
#if SRK_COMPILER == SRK_COMPILER_CLANG
			Compiler::CLANG;
#elif SRK_COMPILER == SRK_COMPILER_GCC
			Compiler::GCC;
#elif SRK_COMPILER == SRK_COMPILER_MSVC
			Compiler::MSVC;
#else
			Compiler::UNKNOWN;
#endif

		enum class OperatingSystem : uint8_t {
			UNKNOWN,
			ANDROID,
			IOS,
			LINUX,
			MACOS,
			WINDOWS
		};

		static constexpr OperatingSystem OPERATING_SYSTEM =
#if SRK_OS == SRK_OS_ANDROID
			OperatingSystem::ANDROID;
#elif SRK_OS == SRK_OS_IOS
			OperatingSystem::IOS;
#elif SRK_OS == SRK_OS_LINUX
			OperatingSystem::LINUX;
#elif SRK_OS == SRK_OS_MACOS
			OperatingSystem::MACOS;
#elif SRK_OS == SRK_OS_WINDOWS
			OperatingSystem::WINDOWS;
#else
			OperatingSystem::UNKNOWN;
#endif
	};


	using float32_t = float;
	using float64_t = double;
}