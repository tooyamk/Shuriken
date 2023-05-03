#pragma once

#include "srk/Concepts.h"

namespace srk::enum_operators {
	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator&(T e1, T e2) noexcept {
		return (T)(std::to_underlying(e1) & std::to_underlying(e2));
	}
	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator|(T e1, T e2) noexcept {
		return (T)(std::to_underlying(e1) | std::to_underlying(e2));
	}
	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator^(T e1, T e2) noexcept {
		return (T)(std::to_underlying(e1) ^ std::to_underlying(e2));
	}
	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator~(T e) noexcept {
		return (T)(~std::to_underlying(e));
	}
	template<ScopedEnum T>
	inline constexpr T& SRK_CALL operator&=(T& e1, T e2) noexcept {
		(std::underlying_type_t<T>&)e1 &= std::to_underlying(e2);
		return e1;
	}
	template<ScopedEnum T>
	inline constexpr T& SRK_CALL operator|=(T& e1, T e2) noexcept {
		(std::underlying_type_t<T>&)e1 |= std::to_underlying(e2);
		return e1;
	}
	template<ScopedEnum T>
	inline constexpr T& SRK_CALL operator^=(T& e1, T e2) noexcept {
		(std::underlying_type_t<T>&)e1 ^= std::to_underlying(e2);
		return e1;
	}

	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator+(T e1, T e2) noexcept {
		return (T)(std::to_underlying(e1) + std::to_underlying(e2));
	}
	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator-(T e1, T e2) noexcept {
		return (T)(std::to_underlying(e1) - std::to_underlying(e2));
	}
	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator*(T e1, T e2) noexcept {
		return (T)(std::to_underlying(e1) * std::to_underlying(e2));
	}
	template<ScopedEnum T>
	inline constexpr T SRK_CALL operator/(T e1, T e2) noexcept {
		return (T)(std::to_underlying(e1) / std::to_underlying(e2));
	}

	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator+(E e, I i) noexcept {
		return (E)(std::to_underlying(e) + i);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator+(I i, E e) noexcept {
		return (E)(i + std::to_underlying(e));
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator-(E e, I i) noexcept {
		return (E)(std::to_underlying(e) - i);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator-(I i, E e) noexcept {
		return (E)(i - std::to_underlying(e));
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator*(E e, I i) noexcept {
		return (E)(std::to_underlying(e) * i);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator*(I i, E e) noexcept {
		return (E)(i * std::to_underlying(e));
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator/(E e, I i) noexcept {
		return (E)(std::to_underlying(e) / i);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator/(I i, E e) noexcept {
		return (E)(i / std::to_underlying(e));
	}

	/*template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator<=>(E e, I i) noexcept {
		return std::to_underlying(e) <=> i;
	}*/
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator==(E e, I i) noexcept {
		return std::to_underlying(e) == i;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator==(I i, E e) noexcept {
		return i == std::to_underlying(e);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator!=(E e, I i) noexcept {
		return std::to_underlying(e) != i;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator!=(I i, E e) noexcept {
		return i != std::to_underlying(e);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator>(E e, I i) noexcept {
		return std::to_underlying(e) > i;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator>(I i, E e) noexcept {
		return i > std::to_underlying(e);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator<(E e, I i) noexcept {
		return std::to_underlying(e) < i;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator<(I i, E e) noexcept {
		return i < std::to_underlying(e);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator>=(E e, I i) noexcept {
		return std::to_underlying(e) >= i;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator>=(I i, E e) noexcept {
		return i >= std::to_underlying(e);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator<=(E e, I i) noexcept {
		return std::to_underlying(e) <= i;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr auto SRK_CALL operator<=(I i, E e) noexcept {
		return i <= std::to_underlying(e);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator>>(E e, I i) noexcept {
		return (E)(std::to_underlying(e) >> i);
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E SRK_CALL operator<<(E e, I i) noexcept {
		return (E)(std::to_underlying(e) << i);
	}

	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator+=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e += i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator-=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e -= i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator*=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e *= i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator/=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e /= i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator>>=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e >>= i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator<<=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e <<= i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator&=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e &= i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator|=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e |= i;
		return e;
	}
	template<ScopedEnum E, std::integral I>
	inline constexpr E& SRK_CALL operator^=(E& e, I i) noexcept {
		(std::underlying_type_t<E>&)e ^= i;
		return e;
	}
}