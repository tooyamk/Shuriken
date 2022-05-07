#pragma once

#define SRK_ENDIAN_UNKNOWN 0
#define SRK_ENDIAN_LITTLE  1234
#define SRK_ENDIAN_BIG     4321

#ifndef SRK_ENDIAN
#	if defined(__linux__) || defined(__GNU__) || defined(__ANDROID__) || defined(__HAIKU__) || defined(__Fuchsia__) || defined(__EMSCRIPTEN__)
#		include <endian.h>
#	elif defined(_AIX)
#		include <sys/machine.h>
#	elif defined(__sun)
#		include <sys/types.h>
#		define BIG_ENDIAN 4321
#		define LITTLE_ENDIAN 1234
#		if defined(_BIG_ENDIAN)
#			define BYTE_ORDER BIG_ENDIAN
#		else
#			define BYTE_ORDER LITTLE_ENDIAN
#		endif
#	elif defined(__MVS__)
#		define BIG_ENDIAN 4321
#		define LITTLE_ENDIAN 1234
#		define BYTE_ORDER BIG_ENDIAN
#	else
#		if !defined(BYTE_ORDER) && !defined(_WIN32)
#			include <machine/endian.h>
#		endif
#	endif

#	if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#		define SRK_ENDIAN SRK_ENDIAN_BIG
#	else
#		define SRK_ENDIAN SRK_ENDIAN_LITTLE
#	endif
#endif