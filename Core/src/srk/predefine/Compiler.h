#pragma once

#define SRK_COMPILER_UNKNOWN 0
#define SRK_COMPILER_CLANG   1
#define SRK_COMPILER_GCC     2
#define SRK_COMPILER_MSVC    3

#ifndef SRK_COMPILER
#	if defined(__clang__)
#		define SRK_COMPILER SRK_COMPILER_CLANG
#	elif defined(__GNUC__)
#		define SRK_COMPILER SRK_COMPILER_GCC
#	elif defined(_MSC_VER)
#		define SRK_COMPILER SRK_COMPILER_MSVC
#	else
#		define SRK_COMPILER SRK_COMPILER_UNKNOWN
#	endif
#endif