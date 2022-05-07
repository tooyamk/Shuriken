#pragma once

#define SRK_OS_UNKNOWN 0
#define SRK_OS_ANDROID 1
#define SRK_OS_IOS     2
#define SRK_OS_LINUX   3
#define SRK_OS_MACOS   4
#define SRK_OS_WINDOWS 5

#ifndef SRK_OS
#   if defined(__ANDROID__)
#       define SRK_OS SRK_OS_ANDROID
#   elif defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#       define SRK_OS SRK_OS_IOS
#	elif defined(linux) || defined(__linux)
#       define SRK_OS SRK_OS_LINUX
#   elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#       define SRK_OS SRK_OS_MACOS
#	elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#       define SRK_OS SRK_OS_WINDOWS
#   else
#       define SRK_OS SRK_OS_UNKNOWN
#   endif
#endif