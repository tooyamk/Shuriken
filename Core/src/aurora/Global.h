#pragma once

#pragma warning(disable : 4244)
#pragma warning(disable : 4251)
#pragma warning(disable : 4267)
#pragma warning(disable : 4275)
#pragma warning(disable : 4838)
#pragma warning(disable : 4996)
#pragma warning(disable : 26812)


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

#undef AE_CPP_VER
#define AE_CPP_VER AE_CPP_VER_UNKNOWN

#ifdef __cplusplus
#	undef __ae_tmp_cpp_ver
#	if AE_COMPILER == AE_COMPILER_MSVC
#		if __cplusplus != _MSVC_LANG
#			define __ae_tmp_cpp_ver _MSVC_LANG
#		endif
#	endif
#	if !defined(__ae_tmp_cpp_ver)
#		define __ae_tmp_cpp_ver __cplusplus
#	endif
#	if __ae_tmp_cpp_ver == 199711L
#		undef AE_CPP_VER
#		define AE_CPP_VER AE_CPP_VER_03
#	elif __ae_tmp_cpp_ver == 201103L
#		undef AE_CPP_VER
#		define AE_CPP_VER AE_CPP_VER_14
#	elif __ae_tmp_cpp_ver == 201402L
#		undef AE_CPP_VER
#		define AE_CPP_VER AE_CPP_VER_14
#	elif __ae_tmp_cpp_ver == 201703L || __ae_tmp_cpp_ver == 201704L
#		undef AE_CPP_VER
#		define AE_CPP_VER AE_CPP_VER_17
#	endif
#undef __ae_tmp_cpp_ver
#endif

#if AE_CPP_VER < AE_CPP_VER_17
#	error compile aurora library need c++17
#endif


#if AE_COMPILER == AE_COMPILER_MSVC
	#define AE_CALL __fastcall
#else
	#define AE_CALL
#endif


#define ae_internal_public public


#define AE_CREATE_MODULE_FN_NAME ae_create_module


#define _AE_TO_STRING(str) #str
#define AE_TO_STRING(str) _AE_TO_STRING(str)


#if AE_OS == AE_OS_WIN
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#elif AE_OS == AE_OS_MAC
#	include <unistd.h>
#endif


#if __cpp_lib_to_chars
#	include <charconv>
#endif
#include <iostream>
#include <mutex>
#include <string>


#if defined(DEBUG) || defined(_DEBUG)
	#define AE_DEBUG
#endif


#if AE_OS == AE_OS_WIN
#	define AE_DLL_EXPORT __declspec(dllexport)
#	define AE_DLL_IMPORT __declspec(dllimport)
#else
#	define AE_DLL_EXPORT
#	define AE_DLL_IMPORT
//#	define AE_DLL_EXPORT __attribute__((dllexport))
//#	define AE_DLL_IMPORT __attribute__((dllimport))
#endif

#define AE_MODULE_DLL_EXPORT AE_DLL_EXPORT
#define AE_MODULE_DLL_IMPORT AE_DLL_IMPORT

#define AE_TEMPLATE_DLL_EXPORT AE_DLL_EXPORT
#define AE_TEMPLATE_DLL_IMPORT

#define AE_EXTENSION_DLL_EXPORT AE_DLL_EXPORT
#define AE_EXTENSION_DLL_IMPORT AE_DLL_IMPORT

#ifdef AE_EXPORTS
#	define AE_DLL AE_DLL_EXPORT
#	define AE_TEMPLATE_DLL AE_TEMPLATE_DLL_EXPORT
#else
#	define AE_DLL AE_DLL_IMPORT
#	define AE_TEMPLATE_DLL AE_TEMPLATE_DLL_IMPORT
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


#define AE_DEFINE_ENUM_BIT_OPERATIION(__ENUM__) \
inline constexpr __ENUM__ AE_CALL operator&(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((std::underlying_type<__ENUM__>::type)e1 & (std::underlying_type<__ENUM__>::type)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator|(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((std::underlying_type<__ENUM__>::type)e1 | (std::underlying_type<__ENUM__>::type)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator^(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((std::underlying_type<__ENUM__>::type)e1 ^ (std::underlying_type<__ENUM__>::type)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator~(__ENUM__ e) { \
	return (__ENUM__)(~(std::underlying_type<__ENUM__>::type)e); \
} \
inline constexpr __ENUM__& AE_CALL operator&=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 & e2; \
	return e1; \
} \
inline constexpr __ENUM__& AE_CALL operator|=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 | e2; \
	return e1; \
} \
inline constexpr __ENUM__& AE_CALL operator^=(__ENUM__& e1, __ENUM__ e2) { \
	e1 = e1 ^ e2; \
	return e1; \
}


namespace std {
	union EndianTest {
		struct {
			bool isLittle;
			uint8_t fill;
		};
		uint16_t value;
	};
	constexpr EndianTest _endianTest = { (uint16_t)1 };
	enum class endian {
		little = 0,
		big = 1,
		native = _endianTest.isLittle ? little : big
	};
}


namespace aurora {
	using f32 = float;
	using f64 = double;


	template<bool Test, class T = void, class F = void>
	struct if_else {};

	template<class T, class F>
	struct if_else<true, T, F> { typedef T type; };

	template<class T, class F>
	struct if_else<false, T, F> { typedef F type; };

	template<bool Test, class T = void, class F = void>
	using if_else_t = typename if_else<Test, T, F>::type;


	template<typename T>
	constexpr bool is_string_v = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

	template<typename T>
	constexpr bool is_wstring_v = std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view>;


	template<size_t Bits> using int_t = if_else_t<Bits >= 0 && Bits <= 8, int8_t, if_else_t<Bits >= 9 && Bits <= 16, int16_t, if_else_t<Bits >= 17 && Bits <= 32, int32_t, if_else_t<Bits >= 33 && Bits <= 64, int64_t, void>>>>;
	template<size_t Bits> using uint_t = if_else_t<Bits >= 0 && Bits <= 8, uint8_t, if_else_t<Bits >= 9 && Bits <= 16, uint16_t, if_else_t<Bits >= 17 && Bits <= 32, uint32_t, if_else_t<Bits >= 33 && Bits <= 64, uint64_t, void>>>>;


	template<typename T>
	struct Recognitor {};


	template<size_t Bits>
	inline constexpr uint_t<Bits> AE_CALL _uintMax() {
		static_assert(Bits <= 64, "error");

		uint_t<Bits> val = 0;
		for (size_t i = 0; i < Bits; ++i) val |= (uint_t<Bits>)1 << i;
		return val;
	}

	template<size_t Bits>
	inline constexpr int_t<Bits> AE_CALL _intMax() {
		return _uintMax<Bits>() >> 1;
	}

	template<size_t Bits>
	inline constexpr int_t<Bits> AE_CALL _intMin() {
		return -_intMax<Bits>() - 1;
	}
	

	template<size_t Bits>
	struct BitInt {
		static constexpr int_t<Bits> MIN = _intMin<Bits>();
		static constexpr int_t<Bits> MAX = _intMax<Bits>();
	};

	template<size_t Bits>
	struct BitUInt {
		static constexpr int_t<Bits> MAX = _uintMax<Bits>();
	};


	template<size_t Bytes>
	inline uint_t<Bytes * 8> AE_CALL byteswap(const uint8_t* data) {
		using t = uint_t<Bytes * 8>;

		if constexpr (Bytes == 0) {
			return 0;
		} else if constexpr (Bytes == 1) {
			return data[0];
		} else if constexpr (Bytes == 2) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _byteswap_ushort(*((t*)data));
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
			return __builtin_bswap16(*((t*)data));
#else
			return (t)data[0] << 8 | (t)data[1];
#endif
		} else if constexpr (Bytes == 3) {
			return (t)data[0] << 16 | (t)data[1] << 8 | (t)data[2];
		} else if constexpr (Bytes == 4) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _byteswap_ulong(*((t*)data));
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
			return __builtin_bswap32(*((t*)data));
#else
			return (t)data[0] << 32 | (t)data[1] << 16 | (t)data[2] << 8 | (t)data[3];
#endif
		} else if constexpr (Bytes == 5) {
			return (t)data[0] << 32 | (t)data[1] << 24 | (t)data[2] << 16 | (t)data[3] << 8 | (t)data[4];
		} else if constexpr (Bytes == 6) {
			return (t)data[0] << 40 | (t)data[1] << 32 | (t)data[2] << 24 | (t)data[3] << 16 | (t)data[4] << 8 | (t)data[5];
		} else if constexpr (Bytes == 7) {
			return (t)data[0] << 48 | (t)data[1] << 40 | (t)data[2] << 32 | (t)data[3] << 24 | (t)data[4] << 16 | (t)data[5] << 8 | (t)data[6];
		} else if constexpr (Bytes == 8) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _byteswap_uint64(*((t*)data));
#elif AE_COMPILER == AE_COMPILER_GCC || AE_COMPILER == AE_COMPILER_CLANG
			return __builtin_bswap64(*((t*)data));
#else
			return (t)data[0] << 56 | (t)data[1] << 48 | (t)data[2] << 40 | (t)data[3] << 32 | (t)data[4] << 24 | (t)data[5] << 16 | (t)data[6] << 8 | (t)data[7];
#endif
		} else {
			static_assert(Bytes <= 8, "not support > 8 Bytes value");
		}
	}

	template<size_t Bytes>
	inline uint_t<Bytes * 8> AE_CALL byteswap(uint_t<Bytes * 8> val) {
		return byteswap<Bytes>((uint8_t*)&val);
	}

	template<typename F, typename = typename std::enable_if_t<std::is_floating_point_v<F>, F>>
	inline F AE_CALL byteswap(F val) {
		auto v = byteswap<sizeof(F)>((uint8_t*)&val);
		return *(F*)&v;
	}


	template<typename T, typename = typename std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>>
	inline T AE_CALL rotl(T val, size_t shift) {
		if constexpr (std::is_same_v<T, uint8_t>) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _rotl8(val, shift);
#else
			shift &= (size_t)0x7;
			return val << shift | val >> (8 - shift);
#endif
		} else if constexpr (std::is_same_v<T, uint16_t>) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _rotl16(val, shift);
#else
			shift &= (size_t)0xF;
			return val << shift | val >> (16 - shift);
#endif
		} else if constexpr (std::is_same_v<T, uint32_t>) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _rotl(val, shift);
#else
			shift &= (size_t)0x1F;
			return val << shift | val >> (32 - shift);
#endif
		} else if constexpr (std::is_same_v<T, uint64_t>) {
#if AE_COMPILER == AE_COMPILER_MSVC
			return _rotl64(val, shift);
#else
			shift &= (size_t)0x3F;
			return val << shift | val >> (64 - shift);
#endif
		} else {
			static_assert(sizeof(T) <= 8, "not support type");
		}
	}


	template<size_t N>
	inline bool AE_CALL memEqual(const void* val1, const void* val2) {
		constexpr size_t n64 = N / sizeof(uint64_t);

		if constexpr (n64 > 0) {
			for (size_t i = 0; i < n64; ++i) {
				if (((uint64_t*)val1)[i] != ((uint64_t*)val2)[i]) return false;
			}
		}

		constexpr size_t offset32 = n64 * sizeof(uint64_t);
		constexpr size_t n32 = n64 ? N % sizeof(uint64_t) : N;

		if constexpr (n32 >= sizeof(uint32_t)) {
			if (*((uint32_t*)(((uint8_t*)val1) + offset32)) != *((uint32_t*)(((uint8_t*)val2) + offset32))) return false;
		}

		constexpr size_t offset16 = n32 >= sizeof(uint32_t) ? offset32 + sizeof(uint32_t) : offset32;
		constexpr size_t n16 = n32 >= sizeof(uint32_t) ? n32 - sizeof(uint32_t) : n32;

		if constexpr (n16 >= sizeof(uint16_t)) {
			if (*((uint16_t*)(((uint8_t*)val1) + offset16)) != *((uint16_t*)(((uint8_t*)val2) + offset16))) return false;
		}

		constexpr size_t offset8 = n16 >= sizeof(uint16_t) ? offset16 + sizeof(uint16_t) : offset16;
		constexpr size_t n8 = n16 >= sizeof(uint16_t) ? n16 - sizeof(uint16_t) : n16;

		if constexpr (n8 > 0) {
			if (*((uint8_t*)(((uint8_t*)val1) + offset8)) != *((uint8_t*)(((uint8_t*)val2) + offset8))) return false;
		}

		return true;
	}


	inline bool AE_CALL _memEqual8(const void* val1, const void* val2, size_t length) {
		if (*(uint8_t*)val1 != *(uint8_t*)val2) return false;
		return true;
	}

	inline bool AE_CALL _memEqual16(const void* val1, const void* val2, size_t length) {
		if (length >= 2) {
			if (*(uint16_t*)val1 != *(uint16_t*)val2) return false;

			length -= 2;
			return length ? _memEqual8(((uint16_t*)val1) + 1, ((uint16_t*)val2) + 1, length) : true;
		} else {
			return _memEqual8(val1, val2, length);
		}
	}

	inline bool AE_CALL _memEqual32(const void* val1, const void* val2, size_t length) {
		if (length >= 4) {
			if (*(uint32_t*)val1 != *(uint32_t*)val2) return false;

			length -= 4;
			return length ? _memEqual16(((uint32_t*)val1) + 1, ((uint32_t*)val2) + 1, length) : true;
		} else {
			return _memEqual16(val1, val2, length);
		}
	}

	inline bool AE_CALL memEqual(const void* val1, const void* val2, size_t length) {
		size_t n64 = length >> 3;
		length &= 0b111;
		if (n64) {
			for (size_t i = 0; i < n64; ++i) {
				if (((uint64_t*)val1)[i] != ((uint64_t*)val2)[i]) return false;
			}

			return length ? _memEqual32(((uint64_t*)val1) + n64, ((uint64_t*)val2) + n64, length) : true;
		} else {
			return length ? _memEqual32(val1, val2, length) : true;
		}
	}


	struct AE_DLL NoInit {};
	inline const NoInit NO_INIT = NoInit();


	inline std::wstring AE_CALL getAppPath() {
#if AE_OS == AE_OS_WIN
		const uint32_t SIZE = 1024;
		wchar_t path[SIZE];
		GetModuleFileNameW(nullptr, path, SIZE);
		wchar_t cDir[500] = L"";
		wchar_t cDrive[100] = L"";
		wchar_t cf[100] = L"";
		wchar_t cExt[50] = L"";
		_wsplitpath_s(path, cDrive, cDir, cf, cExt);

		std::wstring str;
		str += cDrive;
		str += cDir;
		return str;
#endif
		return L"";
	}


	class AE_TEMPLATE_DLL Console {
	private:
		struct Buf {
			Buf() :
				needFree(false),
				pos(0),
				size(0),
				data(nullptr) {
			}

			~Buf() {
				if (needFree) delete[] data;
			}

			bool needFree;
			uint32_t pos;
			uint32_t size;
			wchar_t* data;

			inline void AE_CALL write(const char* buf) {
				if (buf) write(buf, strlen(buf));
			}

			void AE_CALL write(const char* buf, uint32_t size) {
				uint32_t s = 0, d = 0;
				for (; s < size;) {
					if (uint8_t c = buf[s]; c == 0) {
						break;
					} else if ((c & 0x80) == 0) {
						++s;
					} else if ((c & 0xE0) == 0xC0) {
						s += 2;
					} else if ((c & 0xF0) == 0xE0) {
						s += 3;
					} else if ((c & 0xF8) == 0xF0) {
						s += 4;
					} else {
						s += 5;
					}
					++d;
				}
				size = s;

				dilatation(d);

				s = 0;
				while (s < size) {
					if (uint8_t c = buf[s]; (c & 0x80) == 0) {
						data[pos++] = buf[s++];
					} else if ((c & 0xE0) == 0xC0) {
						data[pos++] = ((c & 0x3F) << 6) | (buf[s + 1] & 0x3F);
						s += 2;
					} else if ((c & 0xF0) == 0xE0) {
						data[pos++] = ((c & 0x1F) << 12) | ((buf[s + 1] & 0x3F) << 6) | (buf[s + 2] & 0x3F);
						s += 3;
					} else if ((c & 0xF8) == 0xF0) {
						data[pos++] = ((buf[s + 1] & 0x3F) << 12) | ((buf[s + 2] & 0x3F) << 6) | (buf[s + 3] & 0x3F);
						s += 4;
					} else {
						data[pos++] = ((buf[s + 2] & 0x3F) << 12) | ((buf[s + 3] & 0x3F) << 6) | (buf[s + 4] & 0x3F);
						s += 5;
					}
				}
			}

			inline void AE_CALL write(const wchar_t* buf) {
				if (buf) write(buf, wcslen(buf));
			}

			inline void AE_CALL write(const std::wstring& str) {
				write(str.data(), str.size());
			}

			inline void AE_CALL write(const wchar_t* buf, uint32_t size) {
				dilatation(size);
				for (uint32_t i = 0; i < size; ++i) data[pos++] = buf[i];
			}

			inline void AE_CALL write(wchar_t c) {
				dilatation(1);
				data[pos++] = c;
			}

			inline void AE_CALL unsafeWrite(wchar_t c) {
				data[pos++] = c;
			}

			void AE_CALL dilatation(uint32_t size) {
				if (size > this->size - pos) {
					this->size = pos + size + (size >> 1);
					wchar_t* buf = new wchar_t[this->size];
					memcpy(buf, data, pos * sizeof(wchar_t));
					if (needFree) {
						delete[] data;
					} else {
						needFree = true;
					}
					data = buf;
				}
			}
		};

		inline static std::mutex _mutex;

	public:
		template<bool Lock = true, bool ToConsole = false, typename... Args>
		static void AE_CALL print(Args... args) {
			const uint32_t MAX_LEN = 256;
			wchar_t wbuf[MAX_LEN];
			Buf buf;
			buf.size = MAX_LEN;
			buf.data = wbuf;

			((_print(buf, args)), ...);

			buf.write(L'\0');

			if constexpr (Lock) {
				std::scoped_lock lck(_mutex);
				_output<ToConsole>(buf.data);
			} else {
				_output<ToConsole>(buf.data);
			}
		}

	private:
		template<bool ToConsole>
		static void AE_CALL _output(const wchar_t* data) {
			if constexpr (ToConsole) {
				std::wcout << data;
			} else {
#if AE_OS == AE_OS_WIN
				OutputDebugStringW(data);
#endif
			}
		}

		template<typename T>
		static void AE_CALL _print(Buf& out, const T& value) {
			if constexpr (std::is_null_pointer_v<T>) {
				out.write(L"nullptr");
			} else if constexpr (std::is_convertible_v<T, char const*> || std::is_convertible_v<T, wchar_t const*>) {
				out.write(value);
			} else if constexpr (is_string_v<T> || is_wstring_v<T>) {
				out.write(value.data(), value.size());
			} else if constexpr (std::is_same_v<T, bool>) {
				out.write(value ? L"true" : L"false");
			} else if constexpr (std::is_arithmetic_v<T>) {
#if __cpp_lib_to_chars
				if constexpr (std::is_integral_v<T>) {
					char buf[21];
					auto rst = std::to_chars(buf, buf + sizeof(buf), value);
					if (rst.ec == std::errc()) out.write(buf, rst.ptr - buf);
				} else {
					char buf[33];
					auto rst = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general);
					if (rst.ec == std::errc()) out.write(buf, rst.ptr - buf);
				}
#else
				out.write(std::to_wstring(value));
#endif
			} else if constexpr (std::is_enum_v<T>) {
				out.write(L'[');
				_print(out, (std::underlying_type_t<T>)value);
				out.write(L' ');
				out.write(typeid(T).name());
				out.write(L']');
			} else if constexpr (std::is_pointer_v<T>) {
				static constexpr uint32_t COUNT = sizeof(uintptr_t) << 1;
				static constexpr wchar_t MAP[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };

				auto addr = (uintptr_t)value;
				uint8_t values[COUNT];
				for (uint32_t i = 0, j = 0; i < sizeof(uintptr_t); ++i) {
					uint8_t v = addr & 0xFF;
					values[j++] = (v >> 4) & 0xF;
					values[j++] = v & 0xF;
					addr >>= 8;
				}

				out.write(L"[0x");
				out.dilatation(COUNT + 1);
				for (uint32_t i = 1; i <= COUNT; ++i) out.unsafeWrite(MAP[values[COUNT - i]]);
				out.unsafeWrite(L' ');
				out.write(typeid(T).name());
				out.write(L']');
			} else {
				out.write(L'[');
				out.write(typeid(T).name());
				out.write(L']');
			}
		}
	};

	template<bool Lock = true, bool ToConsole = false, typename... Args>
	inline void AE_CALL print(Args... args) {
		Console::print<Lock, ToConsole>(args...);
	}

	template<bool Lock = true, bool ToConsole = false, typename... Args>
	inline void AE_CALL println(Args... args) {
		Console::print<Lock, ToConsole>(args..., L"\n");
	}
}