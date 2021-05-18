#pragma once

#define AE_SIMD_UNKNOWN       0
#define AE_SIMD_ARM_NEON      1
#define AE_SIMD_X86_MMX       2
#define AE_SIMD_X86_SSE       3
#define AE_SIMD_X86_SSE2      4
#define AE_SIMD_X86_SSE3      5
#define AE_SIMD_X86_SSSE3     6
#define AE_SIMD_X86_AMD_SSE4A 7
#define AE_SIMD_X86_SSE4_1    8
#define AE_SIMD_X86_SSE4_2    9
#define AE_SIMD_X86_AVX       10
#define AE_SIMD_X86_AMD_FMA4  11
#define AE_SIMD_X86_AMD_XOP   12
#define AE_SIMD_X86_FMA       13
#define AE_SIMD_X86_AVX2      14
#define AE_SIMD_X86_MIC       15

#ifndef AE_SIMD
#   if defined(__ARM_NEON__) || defined(__aarch64__) || defined (_M_ARM) || defined (_M_ARM64)
#       define AE_SIMD AE_SIMD_ARM_NEON
#   elif defined(__MIC__)
#       define AE_SIMD AE_SIMD_X86_MIC
#	elif defined(__AVX2__)
#       define AE_SIMD AE_SIMD_X86_AVX2
#	elif defined(__FMA__)
#       define AE_SIMD AE_SIMD_X86_FMA
#	elif defined(__XOP__)
#       define AE_SIMD AE_SIMD_X86_AMD_XOP
#	elif defined(__FMA4__)
#       define AE_SIMD AE_SIMD_X86_AMD_FMA4
#   elif defined(__AVX__)
#       define AE_SIMD AE_SIMD_X86_AVX
#	elif defined(__SSE4_2__)
#       define AE_SIMD AE_SIMD_X86_SSE4_2
#	elif defined(__SSE4_1__)
#       define AE_SIMD AE_SIMD_X86_SSE4_1
#	elif defined(__SSE4A__)
#       define AE_SIMD AE_SIMD_X86_AMD_SSE4A
#	elif defined(__SSSE3__)
#       define AE_SIMD AE_SIMD_X86_SSSE3
#	elif defined(__SSE3__)
#       define AE_SIMD AE_SIMD_X86_SSE3
#	elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#       define AE_SIMD AE_SIMD_X86_SSE2
#	elif defined(__SSE__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#       define AE_SIMD AE_SIMD_X86_SSE
#	elif defined(__MMX__)
#       define AE_SIMD AE_SIMD_X86_MMX
#   else
#       define AE_SIMD AE_SIMD_UNKNOWN
#   endif
#endif