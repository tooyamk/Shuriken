#pragma once

#define AE_COMPILER_UNKNOWN 0
#define AE_COMPILER_CLANG   1
#define AE_COMPILER_GCC     2
#define AE_COMPILER_MSVC    3

#ifndef AE_COMPILER
#	if defined(__clang__)
#		define AE_COMPILER AE_COMPILER_CLANG
#	elif defined(__GNUC__)
#		define AE_COMPILER AE_COMPILER_GCC
#	elif defined(_MSC_VER)
#		define AE_COMPILER AE_COMPILER_MSVC
#	else
#		define AE_COMPILER AE_COMPILER_UNKNOWN
#	endif
#endif