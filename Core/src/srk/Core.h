#pragma once

#include "srk/Std.h"

#if SRK_CPP_VER < SRK_CPP_VER_20
#	error compile srk library need c++20
#endif


#define srk_internal_public public


#define _SRK_TO_STRING(str) #str
#define SRK_TO_STRING(str) _SRK_TO_STRING(str)


#if __has_include(<signal.h>)
#	include <signal.h>
#endif
#if __has_include(<bit>)
#	include <bit>
#endif
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>


#ifdef SRK_CORE_EXPORTS
#	define SRK_CORE_DLL SRK_DLL_EXPORT
#else
#	define SRK_CORE_DLL SRK_DLL_IMPORT
#endif


#ifndef __cpp_lib_destroying_delete
#	error srk library need Destroying operator delete feature
#endif
#ifndef __cpp_concepts
#	error srk library need Concepts feature
#endif

namespace srk {
	class SRK_CORE_DLL Environment {
	public:
		Environment() = delete;

		static constexpr bool IS_DEBUG =
#ifdef SRK_DEBUG
			true;
#else
			false;
#endif

		enum class Compiler : uint8_t {
			UNKNOWN,
			CLANG,
			GCC,
			MSVC
		};

		static constexpr Compiler COMPILER =
#if SRK_COMPILER == SRK_COMPILER_CLANG
			Compiler::CLANG;
#elif SRK_COMPILER == SRK_COMPILER_GCC
			Compiler::GCC;
#elif SRK_COMPILER == SRK_COMPILER_MSVC
			Compiler::MSVC;
#else
			Compiler::UNKNOWN;
#endif

		enum class OperatingSystem : uint8_t {
			UNKNOWN,
			ANDROID,
			IOS,
			LINUX,
			MACOS,
			WINDOWS
		};

		static constexpr OperatingSystem OPERATING_SYSTEM =
#if SRK_OS == SRK_OS_ANDROID
			OperatingSystem::ANDROID;
#elif SRK_OS == SRK_OS_IOS
			OperatingSystem::IOS;
#elif SRK_OS == SRK_OS_LINUX
			OperatingSystem::LINUX;
#elif SRK_OS == SRK_OS_MACOS
			OperatingSystem::MACOS;
#elif SRK_OS == SRK_OS_WINDOWS
			OperatingSystem::WINDOWS;
#else
			OperatingSystem::UNKNOWN;
#endif
	};


	using float32_t = float;
	using float64_t = double;


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
		static constexpr size_t BAD_INDEX = (std::numeric_limits<size_t>::max)();
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


	template<typename T, typename... Types> concept SameAnyOf = std::disjunction_v<std::is_same<T, Types>...>;
	template<typename T, typename... Types> struct IsSameAnyOf : std::bool_constant<SameAnyOf<T, Types...>> {};
	template<typename T, typename... Types> using SameAnyOfType = std::enable_if_t<SameAnyOf<T, Types...>, T>;
	template<typename T, typename Tuple> concept SameAnyOfInTuple = TUPLE_FIND_FIRST_VALUE<T, Tuple> != TupleFindFirst<T, Tuple>::BAD_INDEX;
	template<typename T, typename... Types> concept SameAllOf = std::conjunction_v<std::is_same<T, Types>...>;


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
	template<auto Target, auto... Values> concept EqualAnyOf = IsEqualAnyOf<Target, Values...>::value;


	template<typename T, typename... Types> concept ConvertibleAnyOf = std::disjunction_v<std::is_convertible<T, Types>...>;
	template<typename T, typename... Types> struct IsConvertibleAnyOf : std::bool_constant<ConvertibleAnyOf<T, Types...>> {};
	template<typename T, typename... Types> using ConvertibleAnyOfType = std::enable_if_t<ConvertibleAnyOf<T, Types...>, T>;
	template<typename T, typename... Types> concept ConvertibleAllOf = std::conjunction_v<std::is_convertible<Types, T>...>;


	template<typename T> concept Iterable = requires(T & t) { t.begin(); t.end(); };
	template<typename T> struct IsIterable : std::bool_constant<Iterable<T>> {};


	template<typename T> concept ScopedEnum = std::is_scoped_enum_v<T>;
	template<typename T> concept Boolean = std::same_as<T, bool>;
	template<typename T> concept NullPointer = std::is_null_pointer_v<T>;
	template<typename T> concept MemberFunctionPointer = std::is_member_function_pointer_v<T>;
	template<typename Derived, typename Base> concept NullPointerOrDerivedFrom = NullPointer<Derived> || std::derived_from<Derived, Base>;
	template<typename T, typename R, typename... Args> concept InvocableResult = std::is_invocable_r_v<R, T, Args...>;
	template<typename T, typename ResultTuple, typename... Args> concept InvocableAnyOfResult = std::invocable<T, Args...> && SameAnyOfInTuple<std::invoke_result_t<T, Args...>, ResultTuple>;


	template<typename T> struct IsSignedIntegral : std::bool_constant<std::signed_integral<T>> {};
	template<typename T> using SignedIntegralType = std::enable_if_t<std::signed_integral<T>, T>;

	template<typename T> struct IsUnsignedIntegral : std::bool_constant<std::unsigned_integral<T>> {};
	template<typename T> using UnsignedIntegralType = std::enable_if_t<std::unsigned_integral<T>, T>;


	template<typename T> concept Arithmetic = std::is_arithmetic_v<T>;
	template<typename T> using ArithmeticType = std::enable_if_t<Arithmetic<T>, T>;
	template<typename T> using FloatingPointType = std::enable_if_t<std::floating_point<T>, T>;
	template<typename T> using IntegralType = std::enable_if_t<std::integral<T>, T>;


	template<typename T> concept String8 = SameAnyOf<T, std::string, std::u8string>;
	template<typename T> struct IsString8 : std::bool_constant<String8<T>> {};
	template<typename T> using String8Type = std::enable_if_t<String8<T>, T>;

	template<typename T> concept StringData = SameAnyOf<T, std::string, std::string_view>;
	template<typename T> struct IsStringData : std::bool_constant<StringData<T>> {};
	template<typename T> using StringDataType = std::enable_if_t<StringData<T>, T>;

	template<typename T> concept U8StringData = SameAnyOf<T, std::u8string, std::u8string_view>;
	template<typename T> struct IsU8StringData : std::bool_constant<U8StringData<T>> {};
	template<typename T> using U8StringDataType = std::enable_if_t<U8StringData<T>, T>;

	template<typename T> concept ConvertibleStringData = StringData<T> || std::convertible_to<T, char const*>;
	template<typename T> struct IsConvertibleStringData : std::bool_constant<ConvertibleStringData<T>> {};
	template<typename T> using ConvertibleStringDataType = std::enable_if_t<ConvertibleStringData<T>, T>;

	template<typename T> concept ConvertibleU8StringData = U8StringData<T> || std::convertible_to<T, char8_t const*>;
	template<typename T> struct IsConvertibleU8StringData : std::bool_constant<ConvertibleU8StringData<T>> {};
	template<typename T> using ConvertibleU8StringDataType = std::enable_if_t<ConvertibleU8StringData<T>, T>;

	template<typename T> concept ConvertibleString8Data = ConvertibleStringData<T> || ConvertibleU8StringData<T>;
	template<typename T> struct IsConvertibleString8Data : std::bool_constant<ConvertibleString8Data<T>> {};
	template<typename T> using ConvertibleString8DataType = std::enable_if_t<ConvertibleString8Data<T>, T>;

	template<typename T> concept String8Data = StringData<T> || U8StringData<T>;
	template<typename T> struct IsString8Data : std::bool_constant<String8Data<T>> {};
	template<typename T> using String8DataType = std::enable_if_t<String8Data<T>, T>;

	template<typename T> concept String8View = SameAnyOf<T, std::string_view, std::u8string_view>;
	template<typename T> struct IsString8View : std::bool_constant<String8View<T>> {};
	template<typename T> using String8ViewType = std::enable_if_t<String8View<T>, T>;

	template<typename T> using ConvertToString8ViewType = std::enable_if_t<ConvertibleString8Data<T>, std::conditional_t<ConvertibleU8StringData<T>, std::u8string_view, std::string_view>>;
	template<typename T> struct ConvertToString8View { using type = ConvertToString8ViewType<T>; };

	template<typename T> concept ConvertibleString8View = ConvertibleAnyOf<T, std::string_view, std::u8string_view>;
	template<typename T> struct IsConvertibleString8View : std::bool_constant<ConvertibleString8View<T>> {};
	template<typename T> using ConvertibleString8ViewType = std::enable_if_t<ConvertibleString8View<T>, T>;

	template<typename T> concept WStringData = SameAnyOf<T, std::wstring, std::wstring_view>;
	template<typename T> struct IsWStringData : std::bool_constant<WStringData<T>> {};
	template<typename T> using WStringDataType = std::enable_if_t<WStringData<T>, T>;

	template<typename T> concept ConvertibleWStringData = WStringData<T> || std::convertible_to<T, wchar_t const*>;
	template<typename T> struct IsConvertibleWStringData : std::bool_constant<ConvertibleWStringData<T>> {};
	template<typename T> using ConvertibleWStringDataType = std::enable_if_t<ConvertibleWStringData<T>, T>;

	template<typename T> concept String16Data = SameAnyOf<T, std::u16string, std::u16string_view>;
	template<typename T> struct IsString16Data : std::bool_constant<String16Data<T>> {};
	template<typename T> using String16DataType = std::enable_if_t<String16Data<T>, T>;

	template<typename T> concept ConvertibleString16Data = String16Data<T> || std::convertible_to<T, char16_t const*>;
	template<typename T> struct IsConvertibleString16Data : std::bool_constant<ConvertibleString16Data<T>> {};
	template<typename T> using ConvertibleString16DataType = std::enable_if_t<ConvertibleString16Data<T>, T>;

	template<typename T> concept String32Data = SameAnyOf<T, std::u32string, std::u32string_view>;
	template<typename T> struct IsString32Data : std::bool_constant<String32Data<T>> {};
	template<typename T> using String32DataType = std::enable_if_t<String32Data<T>, T>;

	template<typename T> concept ConvertibleString32Data = String32Data<T> || std::convertible_to<T, char32_t const*>;
	template<typename T> struct IsConvertibleString32Data : std::bool_constant<ConvertibleString32Data<T>> {};
	template<typename T> using ConvertibleString32DataType = std::enable_if_t<ConvertibleString32Data<T>, T>;

	template<typename T> concept ConvertibleAnyWideStringData = ConvertibleWStringData<T> || ConvertibleString16Data<T> || ConvertibleString32Data<T>;

	template<typename T> using ConvertToAnyStringType = std::conditional_t<ConvertibleStringData<T>, std::string,
		std::conditional_t<ConvertibleU8StringData<T>, std::u8string,
		std::conditional_t<ConvertibleWStringData<T>, std::wstring,
		std::conditional_t<ConvertibleString16Data<T>, std::u16string,
		std::conditional_t<ConvertibleString32Data<T>, std::u32string, void>
		>>>>;

	template<typename T> using ConvertToAnyStringViewType = std::conditional_t<ConvertibleStringData<T>, std::string_view,
		std::conditional_t<ConvertibleU8StringData<T>, std::u8string_view,
		std::conditional_t<ConvertibleWStringData<T>, std::wstring_view,
		std::conditional_t<ConvertibleString16Data<T>, std::u16string_view,
		std::conditional_t<ConvertibleString32Data<T>, std::u32string_view, void>
		>>>>;


	template<typename L, typename R>
	requires (std::same_as<L, std::string> && (ConvertibleU8StringData<std::remove_cvref_t<R>> || std::same_as<std::remove_cvref_t<R>, char8_t>)) ||
		(std::same_as<L, std::u8string> && (ConvertibleStringData<std::remove_cvref_t<R>> || std::same_as<std::remove_cvref_t<R>, char>))
	inline auto & SRK_CALL operator+=(L & left, R && right) {
		if constexpr (std::same_as<L, std::string>) {
			if constexpr (ConvertibleU8StringData<std::remove_cvref_t<R>>) {
				left += (const std::string_view&)(const ConvertToString8ViewType<std::remove_cvref_t<R>>&)(std::forward<R>(right));
			} else if constexpr (std::same_as<std::remove_cvref_t<R>, char8_t>) {
				left += (char)right;
			}
		} else {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<R>>) {
				left += (const std::u8string_view&)(const ConvertToString8ViewType<std::remove_cvref_t<R>>&)(std::forward<R>(right));
			} else if constexpr (std::same_as<std::remove_cvref_t<R>, char>) {
				left += (char8_t)right;
			}
		}

		return left;
	}


	template<typename L, typename R>
	requires (ConvertibleString8Data<std::remove_cvref_t<L>>&& ConvertibleString8Data<std::remove_cvref_t<R>>) &&
		(((ConvertibleU8StringData<std::remove_cvref_t<L>> || ConvertibleU8StringData<std::remove_cvref_t<R>>) && (ConvertibleStringData<std::remove_cvref_t<L>> || ConvertibleStringData<std::remove_cvref_t<R>>)) ||
		(String8View<std::remove_cvref_t<L>> || String8View<std::remove_cvref_t<R>>))
	inline std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<L>> || ConvertibleU8StringData<std::remove_cvref_t<R>>, std::u8string, std::string> SRK_CALL operator+(L&& left, R&& right) {
		if constexpr (SameAllOf<std::u8string_view, std::remove_cvref_t<L>, std::remove_cvref_t<R>> || SameAllOf<std::string_view, std::remove_cvref_t<L>, std::remove_cvref_t<R>>) {
			std::conditional_t<SameAllOf<std::u8string_view, std::remove_cvref_t<L>, std::remove_cvref_t<R>>, std::u8string, std::string> s;
			s.reserve(left.size() + right.size());
			s += left;
			s += right;
			return std::move(s);
		} else {
			using ConvertTo = std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<L>> || ConvertibleU8StringData<std::remove_cvref_t<R>>, std::u8string_view, std::string_view>;
			return (const ConvertTo&)(const ConvertToString8ViewType<std::remove_cvref_t<L>>&)(std::forward<L>(left)) + (const ConvertTo&)(const ConvertToString8ViewType<std::remove_cvref_t<R>>&)(std::forward<R>(right));
		}
	}


	template<size_t Bits> using int_t = std::enable_if_t<Bits <= 64, std::conditional_t<Bits <= 8, int8_t, std::conditional_t<Bits <= 16, int16_t, std::conditional_t<Bits <= 32, int32_t, int64_t>>>>;
	template<size_t Bits> using uint_t = std::enable_if_t<Bits <= 64, std::conditional_t<Bits <= 8, uint8_t, std::conditional_t<Bits <= 16, uint16_t, std::conditional_t<Bits <= 32, uint32_t, uint64_t>>>>;
	template<size_t Bits> using float_t = std::enable_if_t<Bits <= 64, std::conditional_t<Bits <= 32, float32_t, float64_t>>;


#ifdef __cpp_lib_generic_unordered_lookup
	template<typename T>
	struct TransparentHash {
		using is_transparent = void;

		template<typename K>
		inline size_t SRK_CALL operator()(K&& key) const {
			return std::hash<T>{}(key);
		}
	};

	using QueryString = std::string_view;

	using StringUnorderedSet = std::unordered_set<std::string, TransparentHash<std::string_view>, std::equal_to<>>;

	template<typename T>
	using StringUnorderedMap = std::unordered_map<std::string, T, TransparentHash<std::string_view>, std::equal_to<>>;
#else
	using QueryString = std::string;

	using StringUnorderedSet = std::unordered_set<std::string>;

	template<typename T>
	using StringUnorderedMap = std::unordered_map<std::string, T>;
#endif
}