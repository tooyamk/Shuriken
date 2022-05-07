#pragma once

#define SRK_SIMD_UNKNOWN       0
#define SRK_SIMD_ARM_NEON      1
#define SRK_SIMD_X86_MMX       2
#define SRK_SIMD_X86_SSE       3
#define SRK_SIMD_X86_SSE2      4
#define SRK_SIMD_X86_SSE3      5
#define SRK_SIMD_X86_SSSE3     6
#define SRK_SIMD_X86_AMD_SSE4A 7
#define SRK_SIMD_X86_SSE4_1    8
#define SRK_SIMD_X86_SSE4_2    9
#define SRK_SIMD_X86_AVX       10
#define SRK_SIMD_X86_AMD_FMA4  11
#define SRK_SIMD_X86_AMD_XOP   12
#define SRK_SIMD_X86_FMA       13
#define SRK_SIMD_X86_AVX2      14
#define SRK_SIMD_X86_MIC       15

#ifndef SRK_SIMD
#   if defined(__ARM_NEON__) || defined(__aarch64__) || defined (_M_ARM) || defined (_M_ARM64)
#       define SRK_SIMD SRK_SIMD_ARM_NEON
#   elif defined(__MIC__)
#       define SRK_SIMD SRK_SIMD_X86_MIC
#	elif defined(__AVX2__)
#       define SRK_SIMD SRK_SIMD_X86_AVX2
#	elif defined(__FMA__)
#       define SRK_SIMD SRK_SIMD_X86_FMA
#	elif defined(__XOP__)
#       define SRK_SIMD SRK_SIMD_X86_AMD_XOP
#	elif defined(__FMA4__)
#       define SRK_SIMD SRK_SIMD_X86_AMD_FMA4
#   elif defined(__AVX__)
#       define SRK_SIMD SRK_SIMD_X86_AVX
#	elif defined(__SSE4_2__)
#       define SRK_SIMD SRK_SIMD_X86_SSE4_2
#	elif defined(__SSE4_1__)
#       define SRK_SIMD SRK_SIMD_X86_SSE4_1
#	elif defined(__SSE4A__)
#       define SRK_SIMD SRK_SIMD_X86_AMD_SSE4A
#	elif defined(__SSSE3__)
#       define SRK_SIMD SRK_SIMD_X86_SSSE3
#	elif defined(__SSE3__)
#       define SRK_SIMD SRK_SIMD_X86_SSE3
#	elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#       define SRK_SIMD SRK_SIMD_X86_SSE2
#	elif defined(__SSE__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#       define SRK_SIMD SRK_SIMD_X86_SSE
#	elif defined(__MMX__)
#       define SRK_SIMD SRK_SIMD_X86_MMX
#   else
#       define SRK_SIMD SRK_SIMD_UNKNOWN
#   endif
#endif