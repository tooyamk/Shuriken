#pragma once

#define AE_ARCH_UNKNOWN 0
#define AE_ARCH_ARM     1
#define AE_ARCH_X86     2

#define  AE_ARCH_WORD_BITS_UNKNOWN 0
#define  AE_ARCH_WORD_BITS_32      1
#define  AE_ARCH_WORD_BITS_64      2

#ifndef AE_ARCH
#   if defined(__ARM_ARCH) || defined(__TARGET_ARCH_ARM) || \
       defined(__TARGET_ARCH_THUMB) || defined(_M_ARM) || \
       defined(__arm__) || defined(__arm64) || defined(__thumb__) || \
       defined(_M_ARM64) || defined(__aarch64__) || defined(__AARCH64EL__) || \
       defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || \
       defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || \
       defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || \
       defined(__ARM_ARCH_6KZ__) || defined(__ARM_ARCH_6T2__) || \
       defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__) || \
       defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_4__)
#       if defined(__ARM_ARCH)
#           define AE_ARCH __ARM_ARCH
#       elif defined(__TARGET_ARCH_ARM)
#           define AE_ARCH __TARGET_ARCH_ARM
#       elif defined(__TARGET_ARCH_THUMB)
#           define AE_ARCH __TARGET_ARCH_THUMB
#       elif defined(_M_ARM)
#           define AE_ARCH _M_ARM
#       elif defined(__arm64) || defined(_M_ARM64) || defined(__aarch64__) || defined(__AARCH64EL__)
#           define AE_ARCH 8
#       elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)
#           define AE_ARCH 7
#       elif defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6KZ__) || defined(__ARM_ARCH_6T2__)
#           define AE_ARCH 6
#       elif defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
#           define AE_ARCH 5
#       elif defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_4__)
#           define AE_ARCH 4
#       endif

#       if defined(AE_ARCH)
#           if AE_ARCH < 8
#               define AE_ARCH_WORD_BITS AE_ARCH_WORD_BITS_32
#           else
#               define AE_ARCH_WORD_BITS AE_ARCH_WORD_BITS_64
#           endif
#           define AE_ARCH AE_ARCH_ARM
#       endif
#   elif defined(__x86_64) || defined(__x86_64__) || \
         defined(__amd64__) || defined(__amd64) || \
         defined(_M_X64)
#       define AE_ARCH AE_ARCH_X86
#       define AE_ARCH_WORD_BITS AE_ARCH_WORD_BITS_64
#   elif defined(i386) || defined(__i386__) || \
         defined(__i486__) || defined(__i586__) || \
         defined(__i686__) || defined(__i386) || \
         defined(_M_IX86) || defined(_X86_) || \
         defined(__THW_INTEL__) || defined(__I86__) || \
         defined(__INTEL__)
#       define AE_ARCH AE_ARCH_X86
#       define AE_ARCH_WORD_BITS AE_ARCH_WORD_BITS_32
#   endif

#   ifndef AE_ARCH
#       define AE_ARCH AE_ARCH_UNKNOWN
#       define AE_ARCH_WORD_BITS AE_ARCH_WORD_BITS_UNKNOWN
#   endif
#endif