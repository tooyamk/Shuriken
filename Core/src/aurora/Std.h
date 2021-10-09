#pragma once

#include "aurora/predefine/Architecture.h"
#include "aurora/Config.h"
#include <atomic>
#include <bit>

namespace std {
#ifndef __cpp_lib_endian
	enum class endian {
		little = 0,
		big = 1,
#	if AE_ENDIAN == AE_ENDIAN_BIG
		native = big
#	else
		native = little
#	endif
	};
#endif

#ifndef __cpp_lib_remove_cvref
	template<typename T> using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
	template<typename T> struct remove_cvref { using type = remove_cvref_t<T>; };
#endif

#ifndef __cpp_lib_is_scoped_enum
	template<class>
	struct is_scoped_enum : std::false_type {};

	template<class T>
	requires std::is_enum_v<T>
		struct is_scoped_enum<T> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

	template<typename T> inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;
#endif

#ifndef __cpp_lib_bitops
	template<std::unsigned_integral T>
	[[nodiscard]] inline constexpr T rotl(T x, int32_t s) noexcept {
		constexpr auto bits = sizeof(T) << 3;
		s &= bits - 1;
		return T(x << s) | T(x >> (bits - s));
	}

	template<std::unsigned_integral T>
	[[nodiscard]] inline constexpr T rotr(T x, int32_t s) noexcept {
		constexpr auto bits = sizeof(T) << 3;
		s &= bits - 1;
		return T(x >> s) | T(x << (bits - s));
	}

	template<std::unsigned_integral T>
	inline constexpr bool has_single_bit(T val) noexcept {
		return val != 0 && (val & (val - 1)) == 0;
	}
#endif

#if AE_ARCH_WORD_BITS == AE_ARCH_WORD_BITS_64 && (AE_COMPILER == AE_COMPILER_MSVC || AE_COMPILER == AE_COMPILER_CLANG || AE_COMPILER == AE_COMPILER_GCC)
	template<typename T>
	requires (sizeof(T) == 16)
		class atomic<T> {
		public:
			atomic() {
			}

			atomic(const T& value) :
				_value(value) {
			}

			atomic(T&& value) :
				_value(std::move(value)) {
			}

			inline T load(std::memory_order order = std::memory_order::seq_cst) const {
				T val;
				_compareExchange((volatile T&)_value, val, val);
				return std::move(val);
			}

			inline void store(T desired, std::memory_order order = std::memory_order_seq_cst) {
				T val = _value;
				while (!_compareExchange(_value, val, desired)) {}
			}

			inline T exchange(T desired, std::memory_order order = std::memory_order_seq_cst) {
				T old = _value;
				while (!compare_exchange_strong(old, desired, order)) {}
				return std::move(old);
			}

			inline bool compare_exchange_strong(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) {
				return _compareExchange(_value, expected, desired);
			}

			inline bool compare_exchange_strong(T& expected, T desired, std::memory_order success, std::memory_order failure) {
				return compare_exchange_strong(expected, desired, success);
			}

			inline bool compare_exchange_weak(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) {
				return compare_exchange_strong(expected, desired, order);
			}

			inline bool compare_exchange_weak(T& expected, T desired, std::memory_order success, std::memory_order failure) {
				return compare_exchange_strong(expected, desired, success, failure);
			}

		private:
			T _value;

			inline static bool _compareExchange(volatile T& dst, T& expected, const T& desired) {
#	if AE_ARCH_WORD_BITS == AE_ARCH_WORD_BITS_64
#		if AE_COMPILER == AE_COMPILER_MSVC
				return _InterlockedCompareExchange128((volatile int64_t*)&dst, ((int64_t*)(&desired))[1], ((int64_t*)(&desired))[0], (int64_t*)(&expected));
#		elif AE_COMPILER == AE_COMPILER_CLANG || AE_COMPILER == AE_COMPILER_GCC
				return __sync_bool_compare_and_swap((volatile __uint128_t*)&dst, (__uint128_t&)expected, (const __uint128_t&)desired);
#		endif
#	else
				static std::atomic_flag lock = ATOMIC_FLAG_INIT;

				auto pdst = (volatile uint64_t*)&dst;
				auto pexpected = (uint64_t*)&expected;
				auto pdesired = (const uint64_t*)&desired;
				bool rst;

				while (lock.test_and_set(std::memory_order::acquire)) {}

				if (pdst[0] == pexpected[0] && pdst[1] == pexpected[1]) {
					pdst[0] = pdesired[0];
					pdst[1] = pdesired[1];
					rst = true;
				} else {
					pexpected[0] = pdst[0];
					pexpected[1] = pdst[1];
					rst = false;
				}

				lock.clear(std::memory_order::release);

				return rst;
#	endif
			}
	};
#endif
}