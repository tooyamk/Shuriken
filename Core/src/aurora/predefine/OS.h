#pragma once

#define AE_OS_UNKNOWN 0
#define AE_OS_ANDROID 1
#define AE_OS_IOS     2
#define AE_OS_LINUX   3
#define AE_OS_MACOS   4
#define AE_OS_WINDOWS 5

#ifndef AE_OS
#   if defined(__ANDROID__)
#       define AE_OS AE_OS_ANDROID
#   elif defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#       define AE_OS AE_OS_IOS
#	elif defined(linux) || defined(__linux)
#       define AE_OS AE_OS_LINUX
#   elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#       define AE_OS AE_OS_MACOS
#	elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#       define AE_OS AE_OS_WINDOWS
#   else
#       define AE_OS AE_OS_UNKNOWN
#   endif
#endif