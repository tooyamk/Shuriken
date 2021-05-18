#pragma once

#define AE_ARCH_UNKNOWN 0
#define AE_ARCH_ARM     1
#define AE_ARCH_X86_32  2
#define AE_ARCH_X86_64  3

#ifndef AE_ARCH
#   if defined(__arm__) || defined(__arm64) || defined(__thumb__) || \
       defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) || \
       defined(_M_ARM) || defined(_M_ARM64)
#       define AE_ARCH AE_ARCH_ARM
#   elif defined(i386) || defined(__i386__) || \
         defined(__i486__) || defined(__i586__) || \
         defined(__i686__) || defined(__i386) || \
         defined(_M_IX86) || defined(_X86_) || \
         defined(__THW_INTEL__) || defined(__I86__) || \
         defined(__INTEL__)
#       define AE_ARCH AE_ARCH_X86_32
#   elif defined(__x86_64) || defined(__x86_64__) || \
         defined(__amd64__) || defined(__amd64) || \
         defined(_M_X64)
#       define AE_ARCH AE_ARCH_X86_64
#   else
#       define AE_ARCH AE_ARCH_UNKNOWN
#   endif
#endif