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
}


namespace aurora {
	struct Environment {
#ifdef AE_DEBUG
		static constexpr bool IS_DEBUG = true;
#else
		static constexpr bool IS_DEBUG = false;
#endif

		enum class Compiler : uint8_t {
			UNKNOWN,
			MSVC,
			GCC,
			CLANG
		};


#if AE_COMPILER == AE_COMPILER_MSVC
		static constexpr Compiler COMPILER = Compiler::MSVC;
#elif AE_COMPILER == AE_COMPILER_GCC
		static constexpr Compiler COMPILER = Compiler::GCC;
#elif AE_COMPILER == AE_COMPILER_CLANG
		static constexpr Compiler COMPILER = Compiler::CLANG;
#else
		static constexpr Compiler COMPILER = Compiler::UNKNOWN;
#endif


		enum class OperatingSystem : uint8_t {
			UNKNOWN,
			WINDOWS,
			MAC,
			LINUX,
			IOS,
			ANDROID
		};


#if AE_OS == AE_OS_WIN
		static constexpr OperatingSystem OPERATING_SYSTEM = OperatingSystem::WINDOWS;
#elif AE_OS == AE_OS_MAC
		static constexpr OperatingSystem OPERATING_SYSTEM = OperatingSystem::MAC;
#elif AE_OS == AE_OS_LINUX
		static constexpr OperatingSystem OPERATING_SYSTEM = OperatingSystem::LINUX;
#elif AE_OS == AE_OS_IOS
		static constexpr OperatingSystem OPERATING_SYSTEM = OperatingSystem::IOS;
#elif AE_OS == AE_OS_ANDROID
		static constexpr OperatingSystem OPERATING_SYSTEM = OperatingSystem::ANDROID;
#else
		static constexpr OperatingSystem OPERATING_SYSTEM = OperatingSystem::UNKNOWN;
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
		template<bool IsOutOfBounds, size_t I>
		struct _impl;

		template<size_t I>
		struct _impl<true, I> {
			using type = FailedType;
		};

		template<size_t I>
		struct _impl<false, I> {
			using type = std::tuple_element_t<I, std::tuple<Args...>>;
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
		struct _impl<IsFound, CurValue, OtherValues...> : _impl<Target == CurValue, OtherValues...> {};

	public:
		static constexpr bool value = _impl<false, Values...>::value;
	};
	template<auto Target, auto... Values> concept EqualAnyOf = IsEqualAnyOf<Target, Values...>::value;


	template<typename T, typename... Types> concept ConvertibleAnyOf = std::disjunction_v<std::is_convertible<T, Types>...>;
	template<typename T, typename... Types> struct IsConvertibleAnyOf : std::bool_constant<ConvertibleAnyOf<T, Types...>> {};
	template<typename T, typename... Types> using ConvertibleAnyOfType = std::enable_if_t<ConvertibleAnyOf<T, Types...>, T>;
	template<typename T, typename... Types> concept ConvertibleAllOf = std::conjunction_v<std::is_convertible<Types, T>...>;


	template<typename T> concept ScopedEnum = std::is_scoped_enum_v<T>;
	template<typename T> concept Boolean = std::same_as<T, bool>;
	template<typename T> concept NullPointer = std::is_null_pointer_v<T>;
	template<typename T> concept MemberFunctionPointer = std::is_member_function_pointer_v<T>;
	template<typename Derived, typename Base> concept NullPointerOrDerivedFrom = NullPointer<Derived> || std::derived_from<Derived, Base>;
	template<typename T, typename R, typename... Args> concept InvocableResult = std::is_invocable_r_v<R, T, Args...>;
	template<typename T, typename ResultTuple, typename... Args> concept InvocableAnyOfResult = std::invocable<T, Args...> && SameAnyOfInTuple<std::invoke_result_t<T, Args...>, ResultTuple>;


	template<typename T> struct IsSignedIntegral : std::bool_constant<std::signed_integral<T>>{};
	template<typename T> using SignedIntegralType = std::enable_if_t<std::signed_integral<T>, T>;

	template<typename T> struct IsUnsignedIntegral : std::bool_constant<std::unsigned_integral<T>>{};
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
		template<ScopedEnum T>
		inline constexpr T AE_CALL operator&(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 & (std::underlying_type_t<T>)e2);
		}
		template<ScopedEnum T>
		inline constexpr T AE_CALL operator|(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 | (std::underlying_type_t<T>)e2);
		}
		template<ScopedEnum T>
		inline constexpr T AE_CALL operator^(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 ^ (std::underlying_type_t<T>)e2);
		}
		template<ScopedEnum T>
		inline constexpr T AE_CALL operator~(T e) noexcept {
			return (T)(~(std::underlying_type_t<T>)e);
		}
		template<ScopedEnum T>
		inline constexpr T& AE_CALL operator&=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 &= (std::underlying_type_t<T>)e2;
			return e1;
		}
		template<ScopedEnum T>
		inline constexpr T& AE_CALL operator|=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 |= (std::underlying_type_t<T>)e2;
			return e1;
		}
		template<ScopedEnum T>
		inline constexpr T& AE_CALL operator^=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 ^= (std::underlying_type_t<T>)e2;
			return e1;
		}

		template<ScopedEnum T>
		inline constexpr T AE_CALL operator+(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 + (std::underlying_type_t<T>)e2);
		}
		template<ScopedEnum T>
		inline constexpr T AE_CALL operator-(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 - (std::underlying_type_t<T>)e2);
		}

		template<ScopedEnum E, std::integral I>
		inline constexpr E AE_CALL operator+(E e, I i) noexcept {
			return (E)((std::underlying_type_t<E>)e + i);
		}
		template<ScopedEnum E, std::integral I>
		inline constexpr E AE_CALL operator+(I i, E e) noexcept {
			return (E)(i + (std::underlying_type_t<E>)e);
		}
		template<ScopedEnum E, std::integral I>
		inline constexpr E AE_CALL operator-(E e, I i) noexcept {
			return (E)((std::underlying_type_t<E>)e - i);
		}
		template<ScopedEnum E, std::integral I>
		inline constexpr E AE_CALL operator-(I i, E e) noexcept {
			return (E)(i - (std::underlying_type_t<E>)e);
		}
	}
	

	template<typename L, typename R>
	requires (std::same_as<L, std::string> && (ConvertibleU8StringData<std::remove_cvref_t<R>> || std::same_as<std::remove_cvref_t<R>, char8_t>)) ||
			 (std::same_as<L, std::u8string> && (ConvertibleStringData<std::remove_cvref_t<R>> || std::same_as<std::remove_cvref_t<R>, char>))
	inline auto& AE_CALL operator+=(L& left, R&& right) {
		if constexpr (std::same_as<L, std::string>) {
			if constexpr (ConvertibleU8StringData<std::remove_cvref_t<R>>) {
				left += (const std::string_view&)ConvertToString8ViewType<std::remove_cvref_t<R>>(std::forward<R>(right));
			} else if constexpr (std::same_as<std::remove_cvref_t<R>, char8_t>) {
				left += (char)right;
			}
		} else {
			if constexpr (ConvertibleStringData<std::remove_cvref_t<R>>) {
				left += (const std::u8string_view&)ConvertToString8ViewType<std::remove_cvref_t<R>>(std::forward<R>(right));
			} else if constexpr (std::same_as<std::remove_cvref_t<R>, char>) {
				left += (char8_t)right;
			}
		}

		return left;
	}


	template<typename L, typename R>
	requires (ConvertibleString8Data<std::remove_cvref_t<L>> && ConvertibleString8Data<std::remove_cvref_t<R>>) &&
			  (
				  ((ConvertibleU8StringData<std::remove_cvref_t<L>> || ConvertibleU8StringData<std::remove_cvref_t<R>>) && (ConvertibleStringData<std::remove_cvref_t<L>> || ConvertibleStringData<std::remove_cvref_t<R>>)) ||
				  (String8View<std::remove_cvref_t<L>> || String8View<std::remove_cvref_t<R>>)
			  )
	inline std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<L>> || ConvertibleU8StringData<std::remove_cvref_t<R>>, std::u8string, std::string>  AE_CALL operator+(L&& left, R&& right) {
		if constexpr (SameAllOf<std::u8string_view, std::remove_cvref_t<L>, std::remove_cvref_t<R>> || SameAllOf<std::string_view, std::remove_cvref_t<L>, std::remove_cvref_t<R>>) {
			std::conditional_t<SameAllOf<std::u8string_view, std::remove_cvref_t<L>, std::remove_cvref_t<R>>, std::u8string, std::string> s;
			s.reserve(left.size() + right.size());
			s += left;
			s += right;
			return std::move(s);
		} else {
			using ConvertTo = std::conditional_t<ConvertibleU8StringData<std::remove_cvref_t<L>> || ConvertibleU8StringData<std::remove_cvref_t<R>>, std::u8string_view, std::string_view>;
			return (const ConvertTo&)ConvertToString8ViewType<std::remove_cvref_t<L>>(std::forward<L>(left)) + (const ConvertTo&)ConvertToString8ViewType<std::remove_cvref_t<R>>(std::forward<R>(right));
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
		inline size_t AE_CALL operator()(K&& key) const {
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
	requires (!MemberFunctionPointer<F>)
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


	struct AE_CORE_DLL NoInit {};
	inline constexpr NoInit NO_INIT = NoInit();


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