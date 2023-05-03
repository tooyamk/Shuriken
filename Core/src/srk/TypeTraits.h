#pragma once

#include "srk/Core.h"
#include <numeric>
#include <tuple>

namespace srk {
	template<typename T, typename... Args> struct TupleFindFirst;

	template<typename T, typename... Args>
	struct TupleFindFirst<T, std::tuple<Args...>> {
	private:
		template<size_t I>
		static constexpr size_t _impl() {
			if constexpr (I < std::tuple_size_v<std::tuple<Args...>>) {
				if constexpr (std::same_as<T, std::tuple_element_t<I, std::tuple<Args...>>>) {
					return I;
				} else {
					return _impl<I + 1>();
				}
			} else {
				return BAD_INDEX;
			}
		}

	public:
		static constexpr size_t BAD_INDEX = std::numeric_limits<size_t>::max();
		static constexpr size_t VALUE = _impl<0>();
	};
	template<typename T, typename Tuple>
	inline static constexpr auto TUPLE_FIND_FIRST_VALUE = TupleFindFirst<T, Tuple>::VALUE;


	template<size_t I, typename FailedType, typename... Args> struct TupleTryAt;

	template<size_t I, typename FailedType, typename... Args>
	struct TupleTryAt<I, FailedType, std::tuple<Args...>> {
	private:
		template<bool IsOutOfBounds, size_t I2>
		struct _impl;

		template<size_t I2>
		struct _impl<true, I2> {
			using type = FailedType;
		};

		template<size_t I2>
		struct _impl<false, I2> {
			using type = std::tuple_element_t<I2, std::tuple<Args...>>;
		};

	public:
		using type = typename _impl<I >= sizeof...(Args), I>::type;
	};
	template<size_t I, typename FailedType, typename Tuple>
	using TupleTryAtType = typename TupleTryAt<I, FailedType, Tuple>::type;


	template<typename T, typename... Types> struct IsSameAnyOf : std::bool_constant<std::disjunction_v<std::is_same<T, Types>...>> {};
	template<typename T, typename... Types> using SameAnyOfType = std::enable_if_t<IsSameAnyOf<T, Types...>::value, T>;

	template<typename T, typename... Types> struct IsSameAllOf : std::bool_constant<std::conjunction_v<std::is_same<T, Types>...>> {};


	template<auto Target, auto... Values>
	struct IsEqualAnyOf {
	private:
		static constexpr bool _value() {
			size_t i = 0;
			((i += (size_t)(Target == Values)), ...);
			return i;
		}

	public:
		static constexpr bool value = _value();
	};


	template<typename T, typename... Types> struct IsConvertibleAnyOf : std::bool_constant<std::disjunction_v<std::is_convertible<T, Types>...>> {};
	template<typename T, typename... Types> using ConvertibleAnyOfType = std::enable_if_t<IsConvertibleAnyOf<T, Types...>::value, T>;

	template<typename T, typename... Types> struct IsConvertibleAllOf : std::bool_constant<std::conjunction_v<std::is_convertible<T, Types>...>> {};


	template<typename T> struct IsIterable : std::bool_constant<requires(T& t) { t.begin(); t.end(); }> {};


	template<typename T> struct IsSignedIntegral : std::bool_constant<std::signed_integral<T>> {};
	template<typename T> using SignedIntegralType = std::enable_if_t<std::signed_integral<T>, T>;

	template<typename T> struct IsUnsignedIntegral : std::bool_constant<std::unsigned_integral<T>> {};
	template<typename T> using UnsignedIntegralType = std::enable_if_t<std::unsigned_integral<T>, T>;


	template<typename T> using ArithmeticType = std::enable_if_t<std::is_arithmetic_v<T>, T>;
	template<typename T> using FloatingPointType = std::enable_if_t<std::floating_point<T>, T>;
	template<typename T> using IntegralType = std::enable_if_t<std::integral<T>, T>;
}