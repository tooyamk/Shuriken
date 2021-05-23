#pragma once

#include "aurora/predefine/Compiler.h"
#include "aurora/predefine/Endian.h"
#include "aurora/predefine/OS.h"

#define AE_CPP_VER_UNKNOWN 0
#define AE_CPP_VER_03      1
#define AE_CPP_VER_11      2
#define AE_CPP_VER_14      3
#define AE_CPP_VER_17      4
#define AE_CPP_VER_20      5
#define AE_CPP_VER_HIGHER  6

#ifndef AE_CPP_VER
#	ifdef __cplusplus
#		if AE_COMPILER == AE_COMPILER_MSVC
#			if __cplusplus != _MSVC_LANG
#				define __ae_tmp_cpp_ver _MSVC_LANG
#			endif
#		endif
#		ifndef __ae_tmp_cpp_ver
#			define __ae_tmp_cpp_ver __cplusplus
#		endif
#		if __ae_tmp_cpp_ver > 202002L
#			define AE_CPP_VER AE_CPP_VER_HIGHER
#		elif __ae_tmp_cpp_ver > 201703L
#			define AE_CPP_VER AE_CPP_VER_20
#		elif __ae_tmp_cpp_ver > 201402L
#			define AE_CPP_VER AE_CPP_VER_17
#		elif __ae_tmp_cpp_ver > 201103L
#			define AE_CPP_VER AE_CPP_VER_14
#		elif __ae_tmp_cpp_ver > 199711L
#			define AE_CPP_VER AE_CPP_VER_11
#		elif __ae_tmp_cpp_ver == 199711L
#			define AE_CPP_VER AE_CPP_VER_03
#		else
#			define AE_CPP_VER AE_CPP_VER_UNKNOWN
#		endif
#		undef __ae_tmp_cpp_ver
#	else
#		define AE_CPP_VER AE_CPP_VER_UNKNOWN
#	endif
#endif


#if AE_OS == AE_OS_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#endif

#if __has_include(<windows.h>)
#	include <windows.h>
#endif
#if __has_include(<unistd.h>)
#	include <unistd.h>
#endif


#ifndef AE_DEBUG
#	if defined(DEBUG) || defined(_DEBUG)
#		define AE_DEBUG
#	endif
#endif


#if AE_COMPILER == AE_COMPILER_MSVC
#	define AE_CALL __fastcall

#	define AE_DLL_EXPORT __declspec(dllexport)
#	define AE_DLL_IMPORT __declspec(dllimport)
#elif AE_COMPILER == AE_COMPILER_CLANG
#	define AE_CALL

#	define AE_DLL_EXPORT __attribute__((__visibility__("default")))
#	define AE_DLL_IMPORT
#elif AE_COMPILER == AE_COMPILER_GCC
#	define AE_CALL

#	if AE_OS == AE_OS_WINDOWS
#		define AE_DLL_EXPORT __attribute__((__dllexport__))
#		define AE_DLL_IMPORT __attribute__((__dllimport__))
#	else
#		define AE_DLL_EXPORT __attribute__((__visibility__("default")))
#		define AE_DLL_IMPORT
#	endif
#else
#	define AE_CALL

#	define AE_DLL_EXPORT __attribute__((__visibility__("default")))
#	define AE_DLL_IMPORT
#endif