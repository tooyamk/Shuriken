#pragma once

#include "srk/predefine/Architecture.h"
#include "srk/Config.h"
#include <atomic>
#include <bit>
#include <concepts>

#ifndef __cpp_lib_concepts
#	include <utility>
#	include <functional>
#endif

namespace std {
#ifndef __cpp_lib_concepts
	template<typename From, typename To> concept convertible_to = std::is_convertible_v<From, To>&& requires { static_cast<To>(std::declval<From>()); };
	template<typename T> concept default_initializable = std::is_nothrow_destructible_v<T> && std::is_constructible_v<T> && requires { T{}; };
	template<typename Derived, typename Base> concept derived_from = std::is_base_of_v<Base, Derived>&& std::is_convertible_v<const volatile Derived*, const volatile Base*>;
	template<typename T> concept floating_point = std::is_floating_point_v<T>;
	template<typename T> concept integral = std::is_integral_v<T>;
	template<typename F, typename... Args> concept invocable = requires(F && f, Args&&... args) { std::invoke(std::forward<F>(f), std::forward<Args>(args)...); };
	template<typename T> concept signed_integral = std::is_integral_v<T> && std::is_signed_v<T>;
	template<typename T> concept unsigned_integral = std::is_integral_v<T> && !std::is_signed_v<T>;
#endif

#ifndef __cpp_lib_endian
#	define __cpp_lib_endian 201907L
	enum class endian {
		little = 0,
		big = 1,
#	if SRK_ENDIAN == SRK_ENDIAN_BIG
		native = big
#	else
		native = little
#	endif
	};
#endif

#ifndef __cpp_lib_remove_cvref
#	define __cpp_lib_remove_cvref 201711L
	template<typename T> using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
	template<typename T> struct remove_cvref { using type = remove_cvref_t<T>; };
#endif

#ifndef __cpp_lib_is_scoped_enum
#	define __cpp_lib_is_scoped_enum 202011L
	template<typename>
	struct is_scoped_enum : std::false_type {};

	template<typename T>
	requires std::is_enum_v<T>
	struct is_scoped_enum<T> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

	template<typename T> inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;
#endif

#ifndef __cpp_lib_to_underlying
#	define __cpp_lib_to_underlying 202102L
	template <typename T>
	inline constexpr std::underlying_type_t<T> to_underlying(T e) noexcept {
		return (std::underlying_type_t<T>)e;
	}
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
#endif

/*#ifndef __cpp_lib_int_pow2
    template<std::unsigned_integral T>
    inline constexpr bool has_single_bit(T val) noexcept {
        return val != 0 && (val & (val - 1)) == 0;
    }
#endif*/

/*#ifndef __cpp_lib_byteswap
	template<integral T>
	inline T SRK_CALL byteswap(const T val) {
		if constexpr (sizeof(T) == 1) {
			return val;
		} else if constexpr (sizeof(T) == 2) {
#	if SRK_COMPILER == SRK_COMPILER_MSVC
			return _Byteswap_ushort(val);
#	elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
			return __builtin_bswap16(val);
#	else
			auto data = (const uint8_t*)val;
			return (T)data[0] << 8 | (T)data[1];
#	endif
		} else if constexpr (sizeof(T) == 4) {
#	if SRK_COMPILER == SRK_COMPILER_MSVC
			return _Byteswap_ulong(val);
#	elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
			return __builtin_bswap32(val);
#	else
			auto data = (const uint8_t*)val;
			return (T)data[0] << 24 | (T)data[1] << 16 | (T)data[2] << 8 | (T)data[3];
#	endif
		} else if constexpr (sizeof(T) == 8) {
#	if SRK_COMPILER == SRK_COMPILER_MSVC
			return _Byteswap_uint64(val);
#	elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
			return __builtin_bswap64(val);
#	else
			auto data = (const uint8_t*)val;
			return (T)data[0] << 56 | (T)data[1] << 48 | (T)data[2] << 40 | (T)data[3] << 32 | (T)data[4] << 24 | (T)data[5] << 16 | (T)data[6] << 8 | (T)data[7];
#	endif
		} else {
			static_assert(sizeof(T) == 0, "Unexpected integer size");
		}
	}
#endif*/

#if SRK_ARCH_WORD_BITS == SRK_ARCH_WORD_BITS_64 && (SRK_COMPILER == SRK_COMPILER_MSVC || SRK_COMPILER == SRK_COMPILER_CLANG || SRK_COMPILER == SRK_COMPILER_GCC)
	template<typename T>
	requires (sizeof(T) == 16)
	class atomic<T> {
	public:
		atomic() {}

		atomic(const T& value) :
			_value(value) {}

		atomic(T&& value) :
			_value(std::move(value)) {}

		inline T load(std::memory_order order = std::memory_order::seq_cst) const {
			T val;
			_compareExchange((volatile T&)_value, val, val);
			return val;
		}

		inline void store(T desired, std::memory_order order = std::memory_order_seq_cst) {
			T val = _value;
			while (!_compareExchange(_value, val, desired)) {}
		}

		inline T exchange(T desired, std::memory_order order = std::memory_order_seq_cst) {
			T old = _value;
			while (!compare_exchange_strong(old, desired, order)) {}
			return old;
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

		inline bool is_lock_free() const noexcept {
#	if SRK_ARCH_WORD_BITS == SRK_ARCH_WORD_BITS_64 && (SRK_COMPILER == SRK_COMPILER_MSVC || SRK_COMPILER == SRK_COMPILER_CLANG || SRK_COMPILER == SRK_COMPILER_GCC)
			return true;
#	else
			return false;
#	endif
		}

	private:
		T _value;

		inline static bool _compareExchange(volatile T& dst, T& expected, const T& desired) {
#	if SRK_ARCH_WORD_BITS == SRK_ARCH_WORD_BITS_64
#		if SRK_COMPILER == SRK_COMPILER_MSVC
			return _InterlockedCompareExchange128((volatile int64_t*)&dst, ((int64_t*)(&desired))[1], ((int64_t*)(&desired))[0], (int64_t*)(&expected));
#		elif SRK_COMPILER == SRK_COMPILER_CLANG || SRK_COMPILER == SRK_COMPILER_GCC
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
