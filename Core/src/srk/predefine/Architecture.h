#pragma once

#define SRK_ARCH_UNKNOWN 0
#define SRK_ARCH_ARM     1
#define SRK_ARCH_X86     2

#define  SRK_ARCH_WORD_BITS_UNKNOWN 0
#define  SRK_ARCH_WORD_BITS_32      1
#define  SRK_ARCH_WORD_BITS_64      2

#ifndef SRK_ARCH
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
#           define SRK_ARCH __ARM_ARCH
#       elif defined(__TARGET_ARCH_ARM)
#           define SRK_ARCH __TARGET_ARCH_ARM
#       elif defined(__TARGET_ARCH_THUMB)
#           define SRK_ARCH __TARGET_ARCH_THUMB
#       elif defined(_M_ARM)
#           define SRK_ARCH _M_ARM
#       elif defined(__arm64) || defined(_M_ARM64) || defined(__aarch64__) || defined(__AARCH64EL__)
#           define SRK_ARCH 8
#       elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)
#           define SRK_ARCH 7
#       elif defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6KZ__) || defined(__ARM_ARCH_6T2__)
#           define SRK_ARCH 6
#       elif defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
#           define SRK_ARCH 5
#       elif defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_4__)
#           define SRK_ARCH 4
#       endif

#       if defined(SRK_ARCH)
#           if SRK_ARCH < 8
#               define SRK_ARCH_WORD_BITS SRK_ARCH_WORD_BITS_32
#           else
#               define SRK_ARCH_WORD_BITS SRK_ARCH_WORD_BITS_64
#           endif
#           undef SRK_ARCH
#           define SRK_ARCH SRK_ARCH_ARM
#       endif
#   elif defined(__x86_64) || defined(__x86_64__) || \
         defined(__amd64__) || defined(__amd64) || \
         defined(_M_X64)
#       define SRK_ARCH SRK_ARCH_X86
#       define SRK_ARCH_WORD_BITS SRK_ARCH_WORD_BITS_64
#   elif defined(i386) || defined(__i386__) || \
         defined(__i486__) || defined(__i586__) || \
         defined(__i686__) || defined(__i386) || \
         defined(_M_IX86) || defined(_X86_) || \
         defined(__THW_INTEL__) || defined(__I86__) || \
         defined(__INTEL__)
#       define SRK_ARCH SRK_ARCH_X86
#       define SRK_ARCH_WORD_BITS SRK_ARCH_WORD_BITS_32
#   endif

#   ifndef SRK_ARCH
#       define SRK_ARCH SRK_ARCH_UNKNOWN
#       define SRK_ARCH_WORD_BITS SRK_ARCH_WORD_BITS_UNKNOWN
#   endif
#endif