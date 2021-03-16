#pragma once

#define AE_OS_UNKNOWN 0
#define AE_OS_WIN     1
#define AE_OS_MAC     2
#define AE_OS_LINUX   3
#define AE_OS_IOS     4
#define AE_OS_ANDROID 5

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#	define AE_OS AE_OS_WIN
#elif defined(TARGET_OS_IPHONE)
#	define AE_OS AE_OS_IOS
#elif defined(TARGET_OS_MAC)
#	define AE_OS AE_OS_MAC
#elif defined(ANDROID)
#	define AE_OS AE_OS_ANDROID
#elif defined(linux) || defined(__linux) || defined(__linux__)
#	define AE_OS AE_OS_LINUX
#else
#	define AE_OS AE_OS_UNKNOWN
#endif


#define AE_OS_BIT_UNKNOWN 0
#define AE_OS_BIT_32      1
#define AE_OS_BIT_64      2

#if defined(_M_X64) || defined(_M_AMD64) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(_LP64) || defined(__LP64__) || defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__)
#	define AE_OS_BIT AE_OS_BIT_64
#else
#	define AE_OS_BIT AE_OS_BIT_32
#endif


#define AE_COMPILER_UNKNOWN 0
#define AE_COMPILER_MSVC    1
#define AE_COMPILER_GCC     2
#define AE_COMPILER_CLANG   3

#if defined(_MSC_VER)
#	define AE_COMPILER AE_COMPILER_MSVC
#elif defined(__clang__)
#	define AE_COMPILER AE_COMPILER_CLANG
#elif defined(__GNUC__)
#	define AE_COMPILER AE_COMPILER_GCC
#else
#	define AE_COMPILER AE_COMPILER_UNKNOWN
#endif


#if defined(__linux__) || defined(__GNU__) || defined(__HAIKU__) || defined(__Fuchsia__) || defined(__EMSCRIPTEN__)
#	include <endian.h>
#elif defined(_AIX)
#	include <sys/machine.h>
#elif defined(__sun)
#	include <sys/types.h>
#	define BIG_ENDIAN 4321
#	define LITTLE_ENDIAN 1234
#	if defined(_BIG_ENDIAN)
#		define BYTE_ORDER BIG_ENDIAN
#	else
#		define BYTE_ORDER LITTLE_ENDIAN
#	endif
#elif defined(__MVS__)
#	define BIG_ENDIAN 4321
#	define LITTLE_ENDIAN 1234
#	define BYTE_ORDER BIG_ENDIAN
#else
#	if !defined(BYTE_ORDER) && !defined(_WIN32)
#		include <machine/endian.h>
#	endif
#endif

#define AE_ENDIAN_UNKNOWN 0
#define AE_ENDIAN_LITTLE  1234
#define AE_ENDIAN_BIG     4321

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#	define AE_ENDIAN AE_ENDIAN_BIG
#else
#	define AE_ENDIAN AE_ENDIAN_LITTLE
#endif


#define AE_CPP_VER_UNKNOWN 0
#define AE_CPP_VER_03      1
#define AE_CPP_VER_11      2
#define AE_CPP_VER_14      3
#define AE_CPP_VER_17      4
#define AE_CPP_VER_20      5
#define AE_CPP_VER_HIGHER  6

#ifdef __cplusplus
#	if AE_COMPILER == AE_COMPILER_MSVC
#		if __cplusplus != _MSVC_LANG
#			define __ae_tmp_cpp_ver _MSVC_LANG
#		endif
#	endif
#	ifndef __ae_tmp_cpp_ver
#		define __ae_tmp_cpp_ver __cplusplus
#	endif
#	if __ae_tmp_cpp_ver > 202002L
#		define AE_CPP_VER AE_CPP_VER_HIGHER
#	elif __ae_tmp_cpp_ver > 201703L
#		define AE_CPP_VER AE_CPP_VER_20
#	elif __ae_tmp_cpp_ver > 201402L
#		define AE_CPP_VER AE_CPP_VER_17
#	elif __ae_tmp_cpp_ver > 201103L
#		define AE_CPP_VER AE_CPP_VER_14
#	elif __ae_tmp_cpp_ver > 199711L
#		define AE_CPP_VER AE_CPP_VER_11
#	elif __ae_tmp_cpp_ver == 199711L
#		define AE_CPP_VER AE_CPP_VER_03
#   else
#		define AE_CPP_VER AE_CPP_VER_UNKNOWN
#	endif
#	undef __ae_tmp_cpp_ver
#else
#	define AE_CPP_VER AE_CPP_VER_UNKNOWN
#endif

#if AE_CPP_VER < AE_CPP_VER_20
#	error compile aurora library need c++20
#endif


#define ae_internal_public public


#define AE_CREATE_MODULE_FN_NAME ae_create_module


#define _AE_TO_STRING(str) #str
#define AE_TO_STRING(str) _AE_TO_STRING(str)


#if AE_OS == AE_OS_WIN
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#endif


#if __has_include(<windows.h>)
#	include <windows.h>
#endif
#if __has_include(<unistd.h>)
#	include <unistd.h>
#endif

#if __has_include(<bit>)
#	include <bit>
#endif
#if __has_include(<concepts>)
#	include <concepts>
#endif
#include <filesystem>
#include <iostream>
#include <mutex>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>


#ifndef AE_DEBUG
#	if defined(DEBUG) || defined(_DEBUG)
#		define AE_DEBUG
#	endif
#endif


#if AE_COMPILER == AE_COMPILER_MSVC
#	define AE_CALL __fastcall

#	define AE_DLL_EXPORT __declspec(dllexport)
#	define AE_DLL_IMPORT __declspec(dllimport)
#elif AE_COMPILER == AE_COMPILER_CLANG
#	define AE_CALL

#	define AE_DLL_EXPORT __attribute__((__visibility__("default")))
#	define AE_DLL_IMPORT
#elif AE_COMPILER == AE_COMPILER_GCC
#	define AE_CALL

#	if AE_OS == AE_OS_WIN
#		define AE_DLL_EXPORT __attribute__((__dllexport__))
#		define AE_DLL_IMPORT __attribute__((__dllimport__))
#	else
#		define AE_DLL_EXPORT __attribute__((__visibility__("default")))
#		define AE_DLL_IMPORT
#	endif
#else
#	define AE_CALL

#	define AE_DLL_EXPORT __attribute__((__visibility__("default")))
#	define AE_DLL_IMPORT
#endif

#define AE_MODULE_DLL_EXPORT AE_DLL_EXPORT
#define AE_MODULE_DLL_IMPORT AE_DLL_IMPORT

#define AE_EXTENSION_DLL_EXPORT AE_DLL_EXPORT
#define AE_EXTENSION_DLL_IMPORT AE_DLL_IMPORT

#ifdef AE_CORE_EXPORTS
#	define AE_CORE_DLL AE_DLL_EXPORT
#else
#	define AE_CORE_DLL AE_DLL_IMPORT
#endif

#ifdef AE_FW_EXPORTS
#	define AE_FW_DLL AE_DLL_EXPORT
#else
#	define AE_FW_DLL AE_DLL_IMPORT
#endif

#ifdef AE_MODULE_EXPORTS
#	define AE_MODULE_DLL AE_MODULE_DLL_EXPORT
#else
#	define AE_MODULE_DLL AE_MODULE_DLL_IMPORT
#endif

#ifdef AE_EXTENSION_EXPORTS
#	define AE_EXTENSION_DLL AE_EXTENSION_DLL_EXPORT
#else
#	define AE_EXTENSION_DLL AE_EXTENSION_DLL_IMPORT
#endif


#ifndef __cpp_lib_destroying_delete
#	error aurora library need Destroying operator delete feature
#endif
#ifndef __cpp_concepts
#	error aurora library need Concepts feature
#endif


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
	template<typename T> inline constexpr bool is_scoped_enum_v = std::is_enum_v<T> && !std::convertible_to<T, std::underlying_type_t<T>>;
	template<typename T> struct is_scoped_enum : bool_constant<is_scoped_enum_v<T>> {};
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
}


namespace aurora {
	namespace environment {
#ifdef AE_DEBUG
		inline constexpr bool is_debug = true;
#else
		inline constexpr bool is_debug = false;
#endif

		enum class compiler : uint8_t {
			unknown,
			msvc,
			gcc,
			clang
		};


#if AE_COMPILER == AE_COMPILER_MSVC
		inline constexpr compiler current_compiler = compiler::msvc;
#elif AE_COMPILER == AE_COMPILER_GCC
		inline constexpr compiler current_compiler = compiler::gcc;
#elif AE_COMPILER == AE_COMPILER_CLANG
		inline constexpr compiler current_compiler = compiler::clang;
#else
		inline constexpr compiler current_compiler = compiler::unknown;
#endif


		enum class operating_system : uint8_t {
			unknown,
			windows,
			mac,
			linux,
			ios,
			android
		};


#if AE_OS == AE_OS_WIN
		inline constexpr operating_system current_operating_system = operating_system::windows;
#elif AE_OS == AE_OS_MAC
		inline constexpr operating_system current_operating_system = operating_system::mac;
#elif AE_OS == AE_OS_LINUX
		inline constexpr operating_system current_operating_system = operating_system::linux;
#elif AE_OS == AE_OS_IOS
		inline constexpr operating_system current_operating_system = operating_system::ios;
#elif AE_OS == AE_OS_ANDROID
		inline constexpr operating_system current_operating_system = operating_system::android;
#else
		inline constexpr operating_system current_operating_system = operating_system::unknown;
#endif
	}


	using float32_t = float;
	using float64_t = double;


	template<typename T, template<typename> typename... Modifiers>
	struct pakaged_modification;

	template<typename T>
	struct pakaged_modification<T> { using type = T; };

	template<typename T, template<typename> typename Modifier, template<typename> typename... OtherModifiers>
	struct pakaged_modification<T, Modifier, OtherModifiers...> : pakaged_modification<typename Modifier<T>::type, OtherModifiers...> {};


	template<typename T, template<typename> typename Condition, template<typename> typename... Modifiers> concept packaged_concept = Condition<typename pakaged_modification<T, Modifiers...>::type>::value;


	template<typename T, typename Tuple>
	struct tuple_find_first;

	template<typename T, typename... Args>
	struct tuple_find_first<T, std::tuple<Args...>> {
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
				return bad_index;
			}
		}

	public:
		static constexpr size_t bad_index = (std::numeric_limits<size_t>::max)();
		static constexpr size_t value = _impl<0>();
	};
	template<typename T, typename Tuple>
	inline static constexpr auto tuple_find_first_v = tuple_find_first<T, Tuple>::value;


	template<template<typename...> typename Base, typename Derived>
	struct is_base_of_template {
	private:
		template<template<typename...> typename Base, typename Derived>
		struct _impl {
			template<typename... Args> static constexpr std::true_type test(const Base<Args...>*);
			static constexpr std::false_type test(...);
			using type = decltype(test(std::declval<Derived*>()));
		};

	public:
		static constexpr bool value = _impl<Base, Derived>::type::value;
	};
	template<typename Derived, template<typename...> typename Base> concept derived_from_template = is_base_of_template<Base, Derived>::value;


	template<typename T, typename... Types> concept same_any_of = std::disjunction_v<std::is_same<T, Types>...>;
	template<typename T, typename... Types> struct is_same_any_of : std::bool_constant<same_any_of<T, Types...>> {};
	template<typename T, typename... Types> using same_any_of_t = std::enable_if_t<same_any_of<T, Types...>, T>;
	template<typename T, typename Tuple> concept same_any_of_in_tuple = tuple_find_first_v<T, Tuple> != tuple_find_first<T, Tuple>::bad_index;


	template<auto Target, auto... Values>
	struct is_equal_any_of {
	private:
		template<bool IsFound, auto... Values>
		struct _impl;

		template<bool IsFound>
		struct _impl<IsFound> {
			static constexpr bool value = IsFound;
		};

		template<auto... Values>
		struct _impl<true, Values...> {
			static constexpr bool value = true;
		};

		template<bool IsFound, auto CurValue, auto... OtherValues>
		struct _impl<IsFound, CurValue, OtherValues...> : _impl<Target == CurValue, OtherValues...> {

		};

	public:
		static constexpr bool value = _impl<false, Values...>::value;
	};
	template<auto Target, auto... Values> concept equal_any_of = is_equal_any_of<Target, Values...>::value;


	template<typename T, typename... Types> concept convertible_any_of = std::disjunction_v<std::is_convertible<T, Types>...>;
	template<typename T, typename... Types> struct is_convertible_any_of : std::bool_constant<convertible_any_of<T, Types...>> {};
	template<typename T, typename... Types> using convertible_any_of_t = std::enable_if_t<convertible_any_of<T, Types...>, T>;
	template<typename To, typename... Types> concept convertible_all_to = std::conjunction_v<std::is_convertible<Types, To>...>;


	template<typename T> concept scoped_enum = std::is_scoped_enum_v<T>;
	template<typename T> concept boolean = std::same_as<T, bool>;
	template<typename T> concept null_pointer = std::is_null_pointer_v<T>;
	template<typename T> concept member_function_pointer = std::is_member_function_pointer_v<T>;
	template<typename Derived, typename Base> concept null_or_derived_from = null_pointer<Derived> || std::derived_from<Derived, Base>;
	template<typename T, typename R, typename... Args> concept invocable_r = std::is_invocable_r_v<R, T, Args...>;
	template<typename T, typename ResultTuple, typename... Args> concept invocable_any_of_r = std::invocable<T, Args...> && same_any_of_in_tuple<std::invoke_result_t<T, Args...>, ResultTuple>;


	template<typename T> struct is_signed_integral : std::bool_constant<std::signed_integral<T>>{};
	template<typename T> using signed_integral_t = std::enable_if_t<std::signed_integral<T>, T>;

	template<typename T> struct is_unsigned_integral : std::bool_constant<std::unsigned_integral<T>>{};
	template<typename T> using unsigned_integral_t = std::enable_if_t<std::unsigned_integral<T>, T>;


	template<typename T> concept arithmetic = std::is_arithmetic_v<T>;
	template<typename T> using arithmetic_t = std::enable_if_t<arithmetic<T>, T>;
	template<typename T> using floating_point_t = std::enable_if_t<std::floating_point<T>, T>;
	template<typename T> using integral_t = std::enable_if_t<std::integral<T>, T>;


	template<typename T> concept string8 = same_any_of<T, std::string, std::u8string>;
	template<typename T> struct is_string8 : std::bool_constant<string8<T>> {};
	template<typename T> using string8_t = std::enable_if_t<string8<T>, T>;

	template<typename T> concept string_data = same_any_of<T, std::string, std::string_view>;
	template<typename T> struct is_string_data : std::bool_constant<string_data<T>> {};
	template<typename T> using string_data_t = std::enable_if_t<string_data<T>, T>;

	template<typename T> concept u8string_data = same_any_of<T, std::u8string, std::u8string_view>;
	template<typename T> struct is_u8string_data : std::bool_constant<u8string_data<T>> {};
	template<typename T> using u8string_data_t = std::enable_if_t<u8string_data<T>, T>;

	template<typename T> concept convertible_string_data = string_data<T> || std::convertible_to<T, char const*>;
	template<typename T> struct is_convertible_string_data : std::bool_constant<convertible_string_data<T>> {};
	template<typename T> using convertible_string_data_t = std::enable_if_t<convertible_string_data<T>, T>;

	template<typename T> concept convertible_u8string_data = u8string_data<T> || std::convertible_to<T, char8_t const*>;
	template<typename T> struct is_convertible_u8string_data : std::bool_constant<convertible_u8string_data<T>> {};
	template<typename T> using convertible_u8string_data_t = std::enable_if_t<convertible_u8string_data<T>, T>;

	template<typename T> concept convertible_string8_data = convertible_string_data<T> || convertible_u8string_data<T>;
	template<typename T> struct is_convertible_string8_data : std::bool_constant<convertible_string8_data<T>> {};
	template<typename T> using convertible_string8_data_t = std::enable_if_t<convertible_string8_data<T>, T>;

	template<typename T> concept string8_data = string_data<T> || u8string_data<T>;
	template<typename T> struct is_string8_data : std::bool_constant<string8_data<T>> {};
	template<typename T> using string8_data_t = std::enable_if_t<string8_data<T>, T>;

	template<typename T> concept string8_view = same_any_of<T, std::string_view, std::u8string_view>;
	template<typename T> struct is_string8_view : std::bool_constant<string8_view<T>> {};
	template<typename T> using string8_view_t = std::enable_if_t<string8_view<T>, T>;

	template<typename T> using convert_to_string8_view_t = std::enable_if_t<convertible_string8_data<T>, std::conditional_t<convertible_u8string_data<T>, std::u8string_view, std::string_view>>;
	template<typename T> struct convert_to_string8_view { using type = convert_to_string8_view_t<T>; };

	template<typename T> concept convertible_string8_view = convertible_any_of<T, std::string_view, std::u8string_view>;
	template<typename T> struct is_convertible_string8_view : std::bool_constant<convertible_string8_view<T>> {};
	template<typename T> using convertible_string8_view_t = std::enable_if_t<convertible_string8_view<T>, T>;

	template<typename T> concept wstring_data = same_any_of<T, std::wstring, std::wstring_view>;
	template<typename T> struct is_wstring_data : std::bool_constant<wstring_data<T>> {};
	template<typename T> using wstring_data_t = std::enable_if_t<wstring_data<T>, T>;

	template<typename T> concept convertible_wstring_data = wstring_data<T> || std::convertible_to<T, wchar_t const*>;
	template<typename T> struct is_convertible_wstring_data : std::bool_constant<convertible_wstring_data<T>> {};
	template<typename T> using convertible_wstring_data_t = std::enable_if_t<convertible_wstring_data<T>, T>;


	namespace literals {
		inline constexpr int8_t operator"" _i8(uint64_t n) noexcept {
			return (int8_t)n;
		}
		inline constexpr uint8_t operator"" _ui8(uint64_t n) noexcept {
			return (uint8_t)n;
		}
		inline constexpr int16_t operator"" _i16(uint64_t n) noexcept {
			return (int16_t)n;
		}
		inline constexpr uint16_t operator"" _ui16(uint64_t n) noexcept {
			return (uint16_t)n;
		}
		inline constexpr int32_t operator"" _i32(uint64_t n) noexcept {
			return (int32_t)n;
		}
		inline constexpr uint32_t operator"" _ui32(uint64_t n) noexcept {
			return (uint32_t)n;
		}
		inline constexpr int64_t operator"" _i64(uint64_t n) noexcept {
			return (int64_t)n;
		}
		inline constexpr uint64_t operator"" _ui64(uint64_t n) noexcept {
			return (uint64_t)n;
		}
	}


	namespace enum_operators {
		template<scoped_enum T>
		inline constexpr T AE_CALL operator&(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 & (std::underlying_type_t<T>)e2);
		}
		template<scoped_enum T>
		inline constexpr T AE_CALL operator|(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 | (std::underlying_type_t<T>)e2);
		}
		template<scoped_enum T>
		inline constexpr T AE_CALL operator^(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 ^ (std::underlying_type_t<T>)e2);
		}
		template<scoped_enum T>
		inline constexpr T AE_CALL operator~(T e) noexcept {
			return (T)(~(std::underlying_type_t<T>)e);
		}
		template<scoped_enum T>
		inline constexpr T& AE_CALL operator&=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 &= (std::underlying_type_t<T>)e2;
			return e1;
		}
		template<scoped_enum T>
		inline constexpr T& AE_CALL operator|=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 |= (std::underlying_type_t<T>)e2;
			return e1;
		}
		template<scoped_enum T>
		inline constexpr T& AE_CALL operator^=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 ^= (std::underlying_type_t<T>)e2;
			return e1;
		}
	}
	

	template<typename L, typename R>
	requires (std::same_as<L, std::string> && (convertible_u8string_data<std::remove_cvref_t<R>> || std::same_as<std::remove_cvref_t<R>, char8_t>)) ||
			 (std::same_as<L, std::u8string> && (convertible_string_data<std::remove_cvref_t<R>> || std::same_as<std::remove_cvref_t<R>, char>))
	inline auto& AE_CALL operator+=(L& left, R&& right) {
		if constexpr (std::same_as<L, std::string>) {
			if constexpr (convertible_u8string_data<std::remove_cvref_t<R>>) {
				left += (const std::string_view&)convert_to_string8_view_t<std::remove_cvref_t<R>>(std::forward<R>(right));
			} else if constexpr (std::same_as<std::remove_cvref_t<R>, char8_t>) {
				left += (char)right;
			}
		} else {
			if constexpr (convertible_string_data<std::remove_cvref_t<R>>) {
				left += (const std::u8string_view&)convert_to_string8_view_t<std::remove_cvref_t<R>>(std::forward<R>(right));
			} else if constexpr (std::same_as<std::remove_cvref_t<R>, char>) {
				left += (char8_t)right;
			}
		}

		return left;
	}


	template<typename L, typename R>
	requires convertible_u8string_data<std::remove_cvref_t<L>> || convertible_u8string_data<std::remove_cvref_t<R>>
	inline std::u8string AE_CALL operator+(L&& left, R&& right) {
		if constexpr (std::same_as<std::remove_cvref_t<L>, std::u8string_view> && std::same_as<std::remove_cvref_t<R>, std::u8string_view>) {
			std::u8string s;
			s.reserve(left.size() + right.size());
			s += left;
			s += right;
			return std::move(s);
		} else {
			return (const std::u8string_view&)convert_to_string8_view_t<std::remove_cvref_t<L>>(std::forward<L>(left)) + (const std::u8string_view&)convert_to_string8_view_t<std::remove_cvref_t<R>>(std::forward<R>(right));
		}
	}
	

	template<size_t Bits> using int_t = std::enable_if_t<Bits <= 64, std::conditional_t<Bits <= 8, int8_t, std::conditional_t<Bits <= 16, int16_t, std::conditional_t<Bits <= 32, int32_t, int64_t>>>>;
	template<size_t Bits> using uint_t = std::enable_if_t<Bits <= 64, std::conditional_t<Bits <= 8, uint8_t, std::conditional_t<Bits <= 16, uint16_t, std::conditional_t<Bits <= 32, uint32_t, uint64_t>>>>;
	template<size_t Bits> using float_t = std::enable_if_t<Bits <= 64, std::conditional_t<Bits <= 32, float32_t, float64_t>>;


#ifdef __cpp_lib_generic_unordered_lookup
	template<typename T>
	struct transparent_hash {
		using is_transparent = void;

		template<typename K>
		inline size_t AE_CALL operator()(K&& key) const {
			return std::hash<T>{}(key);
		}
	};

	using query_string = std::string_view;

	using string_unordered_set = std::unordered_set<std::string, transparent_hash<std::string_view>, std::equal_to<>>;

	template<typename T>
	using string_unordered_map = std::unordered_map<std::string, T, transparent_hash<std::string_view>, std::equal_to<>>;
#else
	using query_string = std::string;

	using string_unordered_set = std::unordered_set<std::string>;

	template<typename T>
	using string_unordered_map = std::unordered_map<std::string, T>;
#endif


	template<typename F, typename T>
	class Invoker {
	public:
		Invoker(F&& fn, T* target) :
			_fn(fn),
			_target(target) {
		}

		inline AE_CALL operator bool() const {
			return _fn && _target;
		}

		template<typename... Args>
		inline decltype(auto) AE_CALL operator()(Args&&... args) const {
			return (_target->*_fn)(std::forward<Args>(args)...);
		}

	private:
		F _fn;
		T* _target;
	};

	template<typename F>
	class Invoker<F, std::nullptr_t> {
	public:
		Invoker(F&& fn) :
			_fn(std::forward<F>(fn)) {
		}

		inline AE_CALL operator bool() const {
			return _fn;
		}

		template<typename... Args>
		inline decltype(auto) AE_CALL operator()(Args&&... args) const {
			return _fn(std::forward<Args>(args)...);
		}

	private:
		F _fn;
	};
	template<typename F>
	requires (!member_function_pointer<F>)
	Invoker(F)->Invoker<F, std::nullptr_t>;


	template<size_t Bits>
	requires (Bits <= 64)
	inline constexpr uint_t<Bits> AE_CALL uintMax() {
		uint_t<Bits> val = 0;
		for (size_t i = 0; i < Bits; ++i) val |= (uint_t<Bits>)1 << i;
		return val;
	}

	template<size_t Bits>
	inline constexpr int_t<Bits> AE_CALL intMax() {
		return uintMax<Bits>() >> 1;
	}

	template<size_t Bits>
	inline constexpr int_t<Bits> AE_CALL intMin() {
		return -intMax<Bits>() - 1;
	}


	template<size_t Bits>
	struct BitInt {
		using Type = int_t<Bits>;
		static constexpr Type MIN = intMin<Bits>();
		static constexpr Type MAX = intMax<Bits>();
	};

	template<size_t Bits>
	struct BitUInt {
		using Type = uint_t<Bits>;
		static constexpr Type MIN = 0;
		static constexpr Type MAX = uintMax<Bits>();
	};


	template<size_t Bytes>
	requires (Bytes <= 8)
	inline uint_t<Bytes * 8> AE_CALL byteswap(const void* val) {
		using T = uint_t<Bytes * 8>;
		auto data = (const uint8_t*)val;

		if constexpr (Bytes == 0) {
			return 0;
		} else if constexpr (Bytes == 1) {
			return data[0];
		} else if constexpr (Bytes == 2) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _byteswap_ushort(*((T*)data));
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
			return __builtin_bswap16(*((T*)data));
#else
			return (T)data[0] << 8 | (T)data[1];
#endif
		} else if constexpr (Bytes == 3) {
			return (T)data[0] << 16 | (T)data[1] << 8 | (T)data[2];
		} else if constexpr (Bytes == 4) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _byteswap_ulong(*((T*)data));
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
			return __builtin_bswap32(*((T*)data));
#else
			return (T)data[0] << 32 | (T)data[1] << 16 | (T)data[2] << 8 | (T)data[3];
#endif
		} else if constexpr (Bytes == 5) {
			return (T)data[0] << 32 | (T)data[1] << 24 | (T)data[2] << 16 | (T)data[3] << 8 | (T)data[4];
		} else if constexpr (Bytes == 6) {
			return (T)data[0] << 40 | (T)data[1] << 32 | (T)data[2] << 24 | (T)data[3] << 16 | (T)data[4] << 8 | (T)data[5];
		} else if constexpr (Bytes == 7) {
			return (T)data[0] << 48 | (T)data[1] << 40 | (T)data[2] << 32 | (T)data[3] << 24 | (T)data[4] << 16 | (T)data[5] << 8 | (T)data[6];
		} else {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _byteswap_uint64(*((T*)data));
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
			return __builtin_bswap64(*((T*)data));
#else
			return (T)data[0] << 56 | (T)data[1] << 48 | (T)data[2] << 40 | (T)data[3] << 32 | (T)data[4] << 24 | (T)data[5] << 16 | (T)data[6] << 8 | (T)data[7];
#endif
		}
	}

	template<size_t Bytes>
	requires (Bytes <= 8)
	inline uint_t<Bytes * 8> AE_CALL byteswap(uint_t<Bytes * 8> val) {
		return byteswap<Bytes>(&val);
	}

	template<std::floating_point F>
	inline F AE_CALL byteswap(F val) {
		auto v = byteswap<sizeof(F)>(&val);
		return *(F*)&v;
	}


	template<size_t Offset = 1>
	requires (Offset != 0)
	inline const void* AE_CALL memFind(const void* data, size_t dataLength, const void* compare, size_t compareLength) {
		if (compareLength) {
			auto buf = (const uint8_t*)data;

			do {
				if (dataLength < compareLength) return nullptr;
				if (!memcmp(buf, compare, compareLength)) return buf;
				if (dataLength < Offset) return nullptr;

				buf += Offset;
				dataLength -= Offset;
			} while (true);
		} else {
			return data;
		}
	}


	struct AE_CORE_DLL no_init {};
	inline constexpr no_init no_init_v = no_init();


	inline std::filesystem::path AE_CALL getAppPath() {
#if AE_OS == AE_OS_WIN
		wchar_t path[FILENAME_MAX] = { 0 };
		auto count = GetModuleFileNameW(nullptr, path, FILENAME_MAX);
		return std::filesystem::path(std::wstring(path, count > 0 ? count : 0));
#else
		char path[FILENAME_MAX];
		auto count = readlink("/proc/self/exe", path, FILENAME_MAX);
		return std::filesystem::path(std::string(path, count > 0 ? count : 0));
#endif
	}
}