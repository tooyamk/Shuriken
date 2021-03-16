#pragma once

#include "aurora/String.h"

#if __has_include(<android/log.h>)
#	include <android/log.h>
#endif

namespace aurora {
	class AE_CORE_DLL Debug {
	public:
		struct AE_CORE_DLL DebuggerOutput {
			inline void AE_CALL operator()(const std::wstring_view& data) const {
#if AE_OS == AE_OS_WIN
				OutputDebugStringW(data.data());
#elif AE_OS == AE_OS_ANDROID
				__android_log_print(ANDROID_LOG_INFO, "Aurora", "%ls", data.data());
#endif
			}
		};

		struct AE_CORE_DLL ConsoleOutput {
			inline void AE_CALL operator()(const std::wstring_view& data) const {
				std::wcout << data.data();
			}
		};

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
				auto [utf8Len, unicodeLen] = String::calcUtf8ToUnicodeLength(std::string_view(buf, size));
				if (unicodeLen != std::wstring::npos) {
					dilatation(unicodeLen);
					pos += String::Utf8ToUnicode(buf, utf8Len, data + pos);
				}
			}

			inline void AE_CALL write(const std::u8string_view& str) {
				write((const char*)str.data(), str.size());
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
					auto buf = new wchar_t[this->size];
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
		template<typename Output, typename... Args>
		static void AE_CALL print(Args&&... args) {
			std::scoped_lock lck(_mutex);

			const uint32_t MAX_LEN = 256;
			wchar_t wbuf[MAX_LEN];
			Buf buf;
			buf.size = MAX_LEN;
			buf.data = wbuf;

			((_print(buf, std::forward<Args>(args))), ...);

			buf.write(L'\0');

			Output out;
			out(std::wstring_view(buf.data, buf.pos - 1));
		}

	private:
		template<typename T>
		static void AE_CALL _print(Buf& out, T&& value) {
			using Type = std::remove_cvref_t<T>;

			if constexpr (null_pointer<Type>) {
				out.write(L"nullptr");
			} else if constexpr (std::convertible_to<Type, wchar_t const*>) {
				out.write(value);
			} else if constexpr (wstring_data<Type>) {
				out.write(value.data(), value.size());
			} else if constexpr (std::convertible_to<Type, char const*>) {
				_print(out, std::string_view(value));
			} else if constexpr (string_data<Type>) {
				out.write(value.data(), value.size());
				//if (String::isUTF8(value.data(), value.size())) {
				//	out.write(value.data(), value.size());
				//} else {
				//	out.write(L"<nonsupported string encode>");
				//}
			} else if constexpr (std::convertible_to<Type, char8_t const*>) {
				_print(out, std::u8string_view(value));
			} else if constexpr (u8string_data<Type>) {
				out.write(value);
			} else if constexpr (std::same_as<Type, bool>) {
				out.write(value ? L"true" : L"false");
			} else if constexpr (arithmetic<Type>) {
				out.write(std::to_wstring(value));
			} else if constexpr (std::is_enum_v<Type>) {
				out.write(L'[');
				_print(out, (std::underlying_type_t<Type>)value);
				out.write(L' ');
				out.write(typeid(T).name());
				out.write(L']');
			} else if constexpr (std::is_pointer_v<Type>) {
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

	template<typename Output, typename... Args>
	inline void AE_CALL print(Args&&... args) {
		Debug::print<Output>(std::forward<Args>(args)...);
	}

	template<typename Output, typename... Args>
	inline void AE_CALL println(Args&&... args) {
		Debug::print<Output>(std::forward<Args>(args)..., L"\n");
	}

	template<typename... Args>
	inline void AE_CALL printd(Args&&... args) {
		Debug::print<Debug::DebuggerOutput>(std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void AE_CALL printdln(Args&&... args) {
		Debug::print<Debug::DebuggerOutput>(std::forward<Args>(args)..., L"\n");
	}

	template<typename... Args>
	inline void AE_CALL printc(Args&&... args) {
		Debug::print<Debug::ConsoleOutput>(std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void AE_CALL printcln(Args&&... args) {
		Debug::print<Debug::ConsoleOutput>(std::forward<Args>(args)..., L"\n");
	}
}