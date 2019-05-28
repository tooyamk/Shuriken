#pragma once

#pragma warning(disable : 4244)
#pragma warning(disable : 4251)
#pragma warning(disable : 4267)
#pragma warning(disable : 4275)
#pragma warning(disable : 4838)
#pragma warning(disable : 4996)

#define AE_OS_PLATFORM_UNKNOWN 0
#define AE_OS_PLATFORM_WIN     1
#define AE_OS_PLATFORM_MAC     2

#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_UNKNOWN

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDLL)
#undef  AE_TARGET_OS_PLATFORM
#define AE_TARGET_OS_PLATFORM  AE_OS_PLATFORM_WIN
#endif


#if defined(_M_X64) || defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(__x86_64)
#define AE_64BITS
#endif


#define AE_CALL __fastcall


#define ae_internal_public public


#define  AE_CREATE_MODULE_FN_NAME ae_create_module


#define _AE_TO_STRING(str) #str
#define AE_TO_STRING(str) _AE_TO_STRING(str)


#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#elif AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_MAC;
#include <unistd.h>
#endif


#include <charconv>
#include <string>


#if defined(DEBUG) || defined(_DEBUG)
#define AE_DEBUG
#endif


#ifdef __GNUC__
#define AE_DLL_EXPORT __attribute__((dllexport))
#define AE_DLL_IMPORT __attribute__((dllimport))
#else
#define AE_DLL_EXPORT __declspec(dllexport)
#define AE_DLL_IMPORT __declspec(dllimport)
#endif

#define AE_MODULE_DLL_EXPORT AE_DLL_EXPORT
#define AE_MODULE_DLL_IMPORT AE_DLL_IMPORT

#define AE_TEMPLATE_DLL_EXPORT AE_DLL_EXPORT
#define AE_TEMPLATE_DLL_IMPORT

#define AE_MODULE_DLL_EXPORT AE_DLL_EXPORT
#define AE_MODULE_DLL_IMPORT AE_DLL_IMPORT

#define AE_EXTENSION_DLL_EXPORT AE_DLL_EXPORT
#define AE_EXTENSION_DLL_IMPORT AE_DLL_IMPORT

#ifdef AE_EXPORTS
#define AE_DLL AE_DLL_EXPORT
#define AE_TEMPLATE_DLL AE_TEMPLATE_DLL_EXPORT
#else
#define AE_DLL AE_DLL_IMPORT
#define AE_TEMPLATE_DLL AE_TEMPLATE_DLL_IMPORT
#endif

#ifdef AE_MODULE_EXPORTS
#define AE_MODULE_DLL AE_MODULE_DLL_EXPORT
#else
#define AE_MODULE_DLL AE_MODULE_DLL_IMPORT
#endif

#ifdef AE_EXTENSION_EXPORTS
#define AE_EXTENSION_DLL AE_EXTENSION_DLL_EXPORT
#else
#define AE_EXTENSION_DLL AE_EXTENSION_DLL_IMPORT
#endif


#define AE_DECLA_CANNOT_INSTANTIATE(__CLASS__) \
__CLASS__() = delete; \
__CLASS__(const __CLASS__&) = delete; \
__CLASS__(__CLASS__&&) = delete; \


#define AE_DEFINE_ENUM_BIT_OPERATIION(__ENUM__) \
inline constexpr __ENUM__ AE_CALL operator&(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 & (ui32)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator|(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 | (ui32)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator^(__ENUM__ e1, __ENUM__ e2) { \
	return (__ENUM__)((ui32)e1 ^ (ui32)e2); \
} \
inline constexpr __ENUM__ AE_CALL operator~(__ENUM__ e) { \
	return (__ENUM__)(~(ui32)e); \
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
} \


namespace aurora {
	using i8 = char;
	using ui8 = unsigned char;
	using i16 = short;
	using ui16 = unsigned short;
	using i32 = int;
	using ui32 = unsigned int;
	using i64 = long long;
	using ui64 = unsigned long long;
	using f32 = float;
	using f64 = double;

#ifdef AE_64BITS
	using uint_ptr = ui64;
#else
	using uint_ptr = ui32;
#endif


	enum class Endian : ui8 {
		BIG,
		LITTLE
	};


	const Endian NATIVE_ENDIAN = *(ui16*)"\0\xFF" < 256 ? Endian::BIG : Endian::LITTLE;


	template <typename From, typename To, typename... R>
	struct are_all_convertible : std::bool_constant<std::is_convertible_v<From, To> && are_all_convertible<To, R...>::value> {
	};

	template <typename From, typename To>
	struct are_all_convertible<From, To> : std::bool_constant<std::is_convertible_v<From, To>> {
	};

	template<class From, class To, typename... R> inline constexpr bool are_all_convertible_v = are_all_convertible<From, To, R...>::value;


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
			ui32 pos;
			ui32 size;
			wchar_t* data;

			inline void AE_CALL write(const i8* buf) {
				write(buf, strlen(buf));
			}

			void AE_CALL write(const i8* buf, ui32 size) {
				ui32 s = 0, d = 0;
				for (; s < size;) {
					if (ui8 c = buf[s]; c == 0) {
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
					if (ui8 c = buf[s]; (c & 0x80) == 0) {
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
				write(buf, wcslen(buf));
			}

			inline void AE_CALL write(const wchar_t* buf, ui32 size) {
				dilatation(size);
				for (ui32 i = 0; i < size; ++i) data[pos++] = buf[i];
			}

			inline void AE_CALL write(wchar_t c) {
				dilatation(1);
				data[pos++] = c;
			}

			inline void AE_CALL unsafeWrite(wchar_t c) {
				data[pos++] = c;
			}

			void AE_CALL dilatation(ui32 size) {
				if (size > this->size - pos) {
					this->size = pos + size + (size >> 1);
					wchar_t* buf = new wchar_t[this->size];
					for (ui32 i = 0; i < pos; ++i) buf[i] = data[i];
					if (needFree) {
						delete[] data;
					} else {
						needFree = true;
					}
					data = buf;
				}
			}
		};

	public:
		template<typename... Args>
		static void AE_CALL print(Args... args) {
			const ui32 MAX_LEN = 256;
			wchar_t wbuf[MAX_LEN];
			Buf buf;
			buf.size = MAX_LEN;
			buf.data = wbuf;

			((_print(buf, args)), ...);

			buf.write(L'\0');

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
			OutputDebugStringW(buf.data);
#endif
		}

	private:
		template<typename T>
		static void AE_CALL _print(Buf& out, const T& value) {
			if constexpr (std::is_null_pointer_v<T>) {
				out.write(L"nullptr");
			} else if constexpr (std::is_convertible_v<T, i8 const*>) {
				out.write(value);
			} else if constexpr (std::is_convertible_v<T, wchar_t const*>) {
				out.write(value);
			} else if constexpr (std::is_same_v<T, std::string>) {
				out.write(value.c_str(), value.size());
			} else if constexpr (std::is_same_v<T, std::wstring>) {
				out.write(value.c_str(), value.size());
			} else if constexpr (std::is_same_v<T, bool>) {
				out.write(value ? L"true" : L"false");
			} else if constexpr (std::is_enum_v<T>) {
				out.write(L'[');
				_print(out, (std::underlying_type_t<T>)value);
				out.write(L' ');
				out.write(typeid(T).name());
				out.write(L']');
			} else if constexpr (std::is_arithmetic_v<T>) {
				if constexpr (std::is_integral_v<T>) {
					i8 buf[21];
					auto rst = std::to_chars(buf, buf + sizeof(buf), value);
					if (rst.ec == std::errc()) out.write(buf, rst.ptr - buf);
				} else {
					i8 buf[33];
					auto rst = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general);
					if (rst.ec == std::errc()) out.write(buf, rst.ptr - buf);
				}
			} else if constexpr (std::is_pointer_v<T>) {
				const ui32 COUNT = sizeof(uint_ptr) << 1;
				const wchar_t MAP[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };

				auto address = (uint_ptr)value;
				ui8 values[COUNT];
				for (ui32 i = 0, j = 0; i < sizeof(uint_ptr); ++i) {
					ui8 v = address & 0xFF;
					values[j++] = (v >> 4) & 0xF;
					values[j++] = v & 0xF;
					address >>= 8;
				}

				out.write(L"[0x");
				out.dilatation(COUNT + 1);
				for (ui32 i = 1; i <= COUNT; ++i) out.unsafeWrite(MAP[values[COUNT - i]]);
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

	template<typename... Args>
	inline void AE_CALL print(Args... args) {
		Console::print(args...);
	}

	template<typename... Args>
	inline void AE_CALL println(Args... args) {
		Console::print(args..., L"\n");
	}
}