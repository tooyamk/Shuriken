#pragma once

#define AE_OS_UNKNOWN 0
#define AE_OS_WIN     1
#define AE_OS_MAC     2
#define AE_OS_LINUX   3
#define AE_OS_IOS     4
#define AE_OS_ANDROID 5

#undef AE_OS
#define AE_OS AE_OS_UNKNOWN

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#	undef AE_OS
#	define AE_OS AE_OS_WIN
#elif defined(TARGET_OS_IPHONE)
#	undef AE_OS
#	define AE_OS AE_OS_IOS
#elif defined(TARGET_OS_MAC)
#	undef AE_OS
#	define AE_OS AE_OS_MAC
#elif defined(ANDROID)
#	undef AE_OS
#	define AE_OS AE_OS_ANDROID
#elif defined(linux) || defined(__linux) || defined(__linux__)
#	undef AE_OS
#	define AE_OS AE_OS_LINUX
#endif


#define AE_OS_BIT_UNKNOWN 0
#define AE_OS_BIT_32      1
#define AE_OS_BIT_64      2

#undef AE_OS_BIT
#define AE_OS_BIT AE_OS_BIT_32

#if defined(_M_X64) || defined(_M_AMD64) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(_LP64) || defined(__LP64__) || defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__)
#	undef AE_OS_BIT
#	define AE_OS_BIT AE_OS_BIT_64
#endif


#define AE_COMPILER_UNKNOWN 0
#define AE_COMPILER_MSVC    1
#define AE_COMPILER_GCC     2
#define AE_COMPILER_CLANG   3

#undef AE_COMPILER
#define AE_COMPILER AE_COMPILER_UNKNOWN

#if defined(_MSC_VER)
#	undef AE_COMPILER
#	define AE_COMPILER AE_COMPILER_MSVC
#elif defined(__clang__)
#	undef AE_COMPILER
#	define AE_COMPILER AE_COMPILER_CLANG
#elif defined(__GNUC__)
#	undef AE_COMPILER
#	define AE_COMPILER AE_COMPILER_GCC
#endif


#define AE_CPP_VER_UNKNOWN 0
#define AE_CPP_VER_03      1
#define AE_CPP_VER_11      2
#define AE_CPP_VER_14      3
#define AE_CPP_VER_17      4
#define AE_CPP_VER_20      5
#define AE_CPP_VER_HIGHER  6

#undef AE_CPP_VER
#define AE_CPP_VER AE_CPP_VER_UNKNOWN

#ifdef __cplusplus
#	undef __ae_tmp_cpp_ver
#	undef AE_CPP_VER
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
#undef __ae_tmp_cpp_ver
#endif

#if AE_CPP_VER < AE_CPP_VER_17
#	error compile aurora library need c++17
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


#define AE_DECLARE_CANNOT_INSTANTIATE(__CLASS__) \
__CLASS__() = delete; \
__CLASS__(const __CLASS__&) = delete; \
__CLASS__(__CLASS__&&) = delete; \
__CLASS__& operator=(const __CLASS__&) = delete; \
__CLASS__& operator=(__CLASS__&&) = delete;


#ifndef __cpp_lib_char8_t
using char8_t = char;
#endif


namespace std {
#ifndef __cpp_lib_char8_t
	using u8string = std::string;
	using u8string_view = std::string_view;
#endif

#ifndef __cpp_lib_endian
	constexpr uint8_t endian_tester() {
		union {
			uint8_t data[2];
			uint16_t value;
		} tester = { (uint16_t)1 };
		return tester.data[0];
	}
	enum class endian {
		little = 0,
		big = 1,
		native = endian_tester() == 1 ? little : big
	};
#endif

#ifndef __cpp_lib_remove_cvref
	template<typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

	template<typename T>
	struct remove_cvref { using type = remove_cvref_t<T>; };
#endif

#ifndef __cpp_lib_is_scoped_enum
	template<typename T>
	struct is_scoped_enum {
		constexpr static bool value = std::is_enum_v<T> && !std::is_convertible_v<T, std::underlying_type_t<T>>;
	};

	template<typename T>
	inline constexpr bool is_scoped_enum_v = std::is_scoped_enum<T>::value;
#endif

#ifndef __cpp_lib_bitops
	template<typename T, typename = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
	[[nodiscard]] inline constexpr T rotl(T x, int32_t s) noexcept {
		constexpr auto bits = sizeof(T) << 3;
		s &= bits - 1;
		return x << s | x >> (bits - s);
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
	[[nodiscard]] inline constexpr T rotr(T x, int32_t s) noexcept {
		constexpr auto bits = sizeof(T) << 3;
		s &= bits - 1;
		return x >> s | x << (bits - s);
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>>
	constexpr bool has_single_bit(T val) noexcept {
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
		template<typename T, typename = std::enable_if_t<std::is_scoped_enum_v<T>>>
		inline constexpr T AE_CALL operator&(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 & (std::underlying_type_t<T>)e2);
		}
		template<typename T, typename = std::enable_if_t<std::is_scoped_enum_v<T>>>
		inline constexpr T AE_CALL operator|(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 | (std::underlying_type_t<T>)e2);
		}
		template<typename T, typename = std::enable_if_t<std::is_scoped_enum_v<T>>>
		inline constexpr T AE_CALL operator^(T e1, T e2) noexcept {
			return (T)((std::underlying_type_t<T>)e1 ^ (std::underlying_type_t<T>)e2);
		}
		template<typename T, typename = std::enable_if_t<std::is_scoped_enum_v<T>>>
		inline constexpr T AE_CALL operator~(T e) noexcept {
			return (T)(~(std::underlying_type_t<T>)e);
		}
		template<typename T, typename = std::enable_if_t<std::is_scoped_enum_v<T>>>
		inline constexpr T& AE_CALL operator&=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 &= (std::underlying_type_t<T>)e2;
			return e1;
		}
		template<typename T, typename = std::enable_if_t<std::is_scoped_enum_v<T>>>
		inline constexpr T& AE_CALL operator|=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 |= (std::underlying_type_t<T>)e2;
			return e1;
		}
		template<typename T, typename = std::enable_if_t<std::is_scoped_enum_v<T>>>
		inline constexpr T& AE_CALL operator^=(T& e1, T e2) noexcept {
			(std::underlying_type_t<T>&)e1 ^= (std::underlying_type_t<T>)e2;
			return e1;
		}
	}


	template<typename T> constexpr bool is_unsigned_integral_v = std::is_integral_v<T> && std::is_unsigned_v<T>;


	template<typename T> using arithmetic_t = std::enable_if_t<std::is_arithmetic_v<T>, T>;
	template<typename T> using floating_point_t = std::enable_if_t<std::is_floating_point_v<T>, T>;
	template<typename T> using integral_t = std::enable_if_t<std::is_integral_v<T>, T>;
	template<typename T> using unsigned_integral_t = std::enable_if_t<is_unsigned_integral_v<T>, T>;


	template<typename T> constexpr bool is_string_data_v = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;
	template<typename T> constexpr bool is_u8string_data_v = std::is_same_v<T, std::u8string> || std::is_same_v<T, std::u8string_view>;
	template<typename T> constexpr bool is_string8_data_v = is_string_data_v<T> || is_u8string_data_v<T>;
	template<typename T> constexpr bool is_wstring_data_v = std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view>;
	template<typename T> using string_data_t = std::enable_if_t<is_string_data_v<T>, T>;
	template<typename T> using u8string_data_t = std::enable_if_t<is_u8string_data_v<T>, T>;
	template<typename T> using string8_data_t = std::enable_if_t<is_string8_data_v<T>, T>;
	template<typename T> using wstring_data_t = std::enable_if_t<is_wstring_data_v<T>, T>;


#ifdef __cpp_lib_char8_t
	template<typename L, typename R, typename = std::enable_if_t<
		((std::is_same_v<L, std::string>) && (is_u8string_data_v<std::remove_cvref_t<R>> || std::is_convertible_v<std::remove_cvref_t<R>, char8_t const*>) || std::is_same_v<std::remove_cvref_t<R>, char8_t>) ||
		((std::is_same_v<L, std::u8string>) && (is_string_data_v<std::remove_cvref_t<R>> || std::is_convertible_v<std::remove_cvref_t<R>, char const*>) || std::is_same_v<std::remove_cvref_t<R>, char>)>>
	inline auto& AE_CALL operator+=(L& left, R&& right) {
		if constexpr (std::is_same_v<L, std::string>) {
			if constexpr (std::is_same_v<std::remove_cvref_t<R>, std::u8string>) {
				left += (const std::string&)right;
			} else if constexpr (std::is_same_v<std::remove_cvref_t<R>, std::u8string_view>) {
				left += (const std::string_view&)right;
			} else if constexpr (std::is_convertible_v<std::remove_cvref_t<R>, char8_t const*>) {
				left += (const char*)right;
			} else if constexpr (std::is_same_v<std::remove_cvref_t<R>, char8_t>) {
				left += (char)right;
			}
		} else {
			if constexpr (std::is_same_v<std::remove_cvref_t<R>, std::string>) {
				left += (const std::u8string&)right;
			} else if constexpr (std::is_same_v<std::remove_cvref_t<R>, std::string_view>) {
				left += (const std::u8string_view&)right;
			} else if constexpr (std::is_convertible_v<std::remove_cvref_t<R>, char const*>) {
				left += (const char8_t*)right;
			} else if constexpr (std::is_same_v<std::remove_cvref_t<R>, char>) {
				left += (char8_t)right;
			}
		}

		return left;
	}

	template<typename L, typename R, typename = std::enable_if_t<
		((is_u8string_data_v<std::remove_cvref_t<L>> && is_string_data_v<std::remove_cvref_t<R>>) || (is_string_data_v<std::remove_cvref_t<L>> && is_u8string_data_v<std::remove_cvref_t<R>>)) ||
		((is_u8string_data_v<std::remove_cvref_t<L>> && std::is_same_v<std::remove_cvref_t<R>, char>) || (std::is_same_v<std::remove_cvref_t<L>, char> && is_u8string_data_v<std::remove_cvref_t<R>>)) ||
		((is_string_data_v<std::remove_cvref_t<L>> && std::is_same_v<std::remove_cvref_t<R>, char8_t>) || (std::is_same_v<std::remove_cvref_t<L>, char8_t> && is_string_data_v<std::remove_cvref_t<R>>)) ||
		((is_u8string_data_v<std::remove_cvref_t<L>> && std::is_convertible_v<std::remove_cvref_t<R>, char const*>) || (std::is_convertible_v<std::remove_cvref_t<L>, char const*> && is_u8string_data_v<std::remove_cvref_t<R>>)) ||
		((is_string_data_v<std::remove_cvref_t<L>> && std::is_convertible_v<std::remove_cvref_t<R>, char8_t const*>) || (std::is_convertible_v<std::remove_cvref_t<L>, char8_t const*> && is_string_data_v<std::remove_cvref_t<R>>))>>
	inline std::u8string AE_CALL operator+(L&& left, R&& right) {
		if constexpr ((is_u8string_data_v<std::remove_cvref_t<L>> && is_string_data_v<std::remove_cvref_t<R>>) || (is_string_data_v<std::remove_cvref_t<L>> && is_u8string_data_v<std::remove_cvref_t<R>>)) {
			std::u8string s;
			s.reserve(left.size() + right.size());
			if constexpr (std::is_same_v<std::remove_cvref_t<L>, std::string>) {
				s += (const std::u8string&)left;
			} else if constexpr (std::is_same_v<std::remove_cvref_t<L>, std::string_view>) {
				s += (const std::u8string_view&)left;
			} else {
				s += left;
			}
			if constexpr (std::is_same_v<std::remove_cvref_t<R>, std::string>) {
				s += (const std::u8string&)right;
			} else if constexpr (std::is_same_v<std::remove_cvref_t<R>, std::string_view>) {
				s += (const std::u8string_view&)right;
			} else {
				s += right;
			}
			return std::move(s);
		} else if constexpr (is_u8string_data_v<std::remove_cvref_t<L>> && std::is_same_v<std::remove_cvref_t<R>, char>) {
			return std::move(left + std::string_view(&right, 1));
		} else if constexpr (std::is_same_v<std::remove_cvref_t<L>, char> && is_u8string_data_v<std::remove_cvref_t<R>>) {
			return std::move(std::string_view(&left, 1) + right);
		} else if constexpr (is_string_data_v<std::remove_cvref_t<L>> && std::is_same_v<std::remove_cvref_t<R>, char8_t>) {
			return std::move(left + std::u8string_view(&right, 1));
		} else if constexpr (std::is_same_v<std::remove_cvref_t<L>, char8_t> && is_string_data_v<std::remove_cvref_t<R>>) {
			return std::move(std::u8string_view(&left, 1) + right);
		} else if constexpr (is_u8string_data_v<std::remove_cvref_t<L>> && std::is_convertible_v<std::remove_cvref_t<R>, char const*>) {
			return std::move(left + std::string_view(std::forward<R>(right)));
		} else if constexpr (std::is_convertible_v<std::remove_cvref_t<L>, char const*> && is_u8string_data_v<std::remove_cvref_t<R>>) {
			return std::move(std::string_view(std::forward<L>(left)) + right);
		} else if constexpr (std::is_convertible_v<std::remove_cvref_t<L>, char8_t const*> && is_string_data_v<std::remove_cvref_t<R>>) {
			return std::move(std::u8string_view(std::forward<L>(left)) + right);
		} else if constexpr (is_string_data_v<std::remove_cvref_t<L>> && std::is_convertible_v<std::remove_cvref_t<R>, char8_t const*>) {
			return std::move(left + std::u8string_view(std::forward<R>(right)));
		} else {
			return std::u8string();
		}
	}
#endif
	

	template<size_t Bits> using int_t = std::conditional_t<Bits >= 0 && Bits <= 8, int8_t, std::conditional_t<Bits >= 9 && Bits <= 16, int16_t, std::conditional_t<Bits >= 17 && Bits <= 32, int32_t, std::conditional_t<Bits >= 33 && Bits <= 64, int64_t, void>>>>;
	template<size_t Bits> using uint_t = std::conditional_t<Bits >= 0 && Bits <= 8, uint8_t, std::conditional_t<Bits >= 9 && Bits <= 16, uint16_t, std::conditional_t<Bits >= 17 && Bits <= 32, uint32_t, std::conditional_t<Bits >= 33 && Bits <= 64, uint64_t, void>>>>;
	template<size_t Bits> using float_t = std::conditional_t<Bits >= 0 && Bits <= 32, float32_t, std::conditional_t<Bits >= 33 && Bits <= 64, float64_t, void>>;

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
	template<typename F, typename = std::enable_if_t<!std::is_member_function_pointer_v<F>>>
	Invoker(F)->Invoker<F, std::nullptr_t>;


	template<size_t Bits, typename = std::enable_if_t<Bits <= 64>>
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


	template<size_t Bytes, typename = std::enable_if_t<Bytes <= 8>>
	inline uint_t<Bytes * 8> AE_CALL byteswap(const uint8_t* data) {
		using T = uint_t<Bytes * 8>;

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
	inline uint_t<Bytes * 8> AE_CALL byteswap(uint_t<Bytes * 8> val) {
		return byteswap<Bytes>((uint8_t*)&val);
	}

	template<typename F, typename = floating_point_t<F>>
	inline F AE_CALL byteswap(F val) {
		auto v = byteswap<sizeof(F)>((uint8_t*)&val);
		return *(F*)&v;
	}


	template<size_t Offset = 1>
	inline void* AE_CALL memFind(void* val1, size_t val1Length, const void* val2, size_t val2Length) {
		if (val2Length) {
			auto inBuf = (uint8_t*)val1;

			for (; val1Length >= Offset; val1Length -= Offset) {
				if (val1Length < val2Length) return nullptr;

				if (!memcmp(inBuf, val2, val2Length)) return inBuf;

				inBuf += Offset;
			}

			return nullptr;
		} else {
			return val1;
		}
	}

	template<size_t Offset = 1>
	inline const void* AE_CALL memFind(const void* val1, size_t val1Length, const void* val2, size_t val2Length) {
		return memFind<Offset>((void*)val1, val1Length, val2, val2Length);
	}


	struct AE_CORE_DLL NoInit {};
	inline const NoInit NO_INIT = NoInit();


	inline std::filesystem::path AE_CALL getAppPath() {
#if AE_OS == AE_OS_WIN
		wchar_t path[FILENAME_MAX] = { 0 };
		auto count = GetModuleFileNameW(nullptr, path, FILENAME_MAX);
		return std::filesystem::path(std::wstring(path, (count > 0) ? count : 0));
#else
		char path[FILENAME_MAX];
		auto count = readlink("/proc/self/exe", path, FILENAME_MAX);
		return std::filesystem::path(std::string(path, (count > 0) ? count : 0));
#endif
	}
}