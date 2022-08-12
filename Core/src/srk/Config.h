#pragma once

#include "srk/predefine/Compiler.h"
#include "srk/predefine/Endian.h"
#include "srk/predefine/OS.h"

#define SRK_CPP_VER_UNKNOWN 0
#define SRK_CPP_VER_03      1
#define SRK_CPP_VER_11      2
#define SRK_CPP_VER_14      3
#define SRK_CPP_VER_17      4
#define SRK_CPP_VER_20      5
#define SRK_CPP_VER_HIGHER  6

#ifndef SRK_CPP_VER
#	ifdef __cplusplus
#		if SRK_COMPILER == SRK_COMPILER_MSVC
#			if __cplusplus != _MSVC_LANG
#				define __srk_tmp_cpp_ver _MSVC_LANG
#			endif
#		endif
#		ifndef __srk_tmp_cpp_ver
#			define __srk_tmp_cpp_ver __cplusplus
#		endif
#		if __srk_tmp_cpp_ver > 202002L
#			define SRK_CPP_VER SRK_CPP_VER_HIGHER
#		elif __srk_tmp_cpp_ver > 201703L
#			define SRK_CPP_VER SRK_CPP_VER_20
#		elif __srk_tmp_cpp_ver > 201402L
#			define SRK_CPP_VER SRK_CPP_VER_17
#		elif __srk_tmp_cpp_ver > 201103L
#			define SRK_CPP_VER SRK_CPP_VER_14
#		elif __srk_tmp_cpp_ver > 199711L
#			define SRK_CPP_VER SRK_CPP_VER_11
#		elif __srk_tmp_cpp_ver == 199711L
#			define SRK_CPP_VER SRK_CPP_VER_03
#		else
#			define SRK_CPP_VER SRK_CPP_VER_UNKNOWN
#		endif
#		undef __srk_tmp_cpp_ver
#	else
#		define SRK_CPP_VER SRK_CPP_VER_UNKNOWN
#	endif
#endif


#if SRK_OS == SRK_OS_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif

#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#endif

#if __has_include(<windows.h>)
#	include <windows.h>
#endif
#if __has_include(<unistd.h>)
#	include <unistd.h>
#endif


#ifndef SRK_DEBUG
#	if defined(DEBUG) || defined(_DEBUG)
#		define SRK_DEBUG
#	endif
#endif


#if SRK_COMPILER == SRK_COMPILER_MSVC
#	define SRK_CALL __fastcall

#	define SRK_DLL_EXPORT __declspec(dllexport)
#	define SRK_DLL_IMPORT __declspec(dllimport)
#elif SRK_COMPILER == SRK_COMPILER_CLANG
#	define SRK_CALL

#	define SRK_DLL_EXPORT __attribute__((__visibility__("default")))
#	define SRK_DLL_IMPORT
#elif SRK_COMPILER == SRK_COMPILER_GNU
#	define SRK_CALL

#	if SRK_OS == SRK_OS_WINDOWS
#		define SRK_DLL_EXPORT __attribute__((__dllexport__))
#		define SRK_DLL_IMPORT __attribute__((__dllimport__))
#	else
#		define SRK_DLL_EXPORT __attribute__((__visibility__("default")))
#		define SRK_DLL_IMPORT
#	endif
#else
#	define SRK_CALL

#	define SRK_DLL_EXPORT __attribute__((__visibility__("default")))
#	define SRK_DLL_IMPORT
#endif