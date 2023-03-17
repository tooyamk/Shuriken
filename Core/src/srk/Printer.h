#pragma once

#include "srk/Global.h"
#include <mutex>

namespace srk {
	class SRK_CORE_DLL Printer {
	public:
		class SRK_CORE_DLL OutputBuffer {
		public:
			OutputBuffer();
			OutputBuffer(wchar_t* data, size_t size, bool needFree);
			~OutputBuffer();

			inline void SRK_CALL write(const char* buf) {
				if (buf) write(buf, strlen(buf));
			}

			inline void SRK_CALL write(const char8_t* buf) {
				if (buf) write((const char*)buf, strlen((const char*)buf));
			}

			void SRK_CALL write(const char* buf, size_t size);

			inline void SRK_CALL write(const std::string& str) {
				write(str.data(), str.size());
			}

			inline void SRK_CALL write(const std::string_view& str) {
				write(str.data(), str.size());
			}

			inline void SRK_CALL write(const std::u8string_view& str) {
				write((const char*)str.data(), str.size());
			}

			inline void SRK_CALL write(const wchar_t* buf) {
				if (buf) write(buf, wcslen(buf));
			}

			inline void SRK_CALL write(const std::wstring& str) {
				write(str.data(), str.size());
			}

			inline void SRK_CALL write(const std::wstring_view& str) {
				write(str.data(), str.size());
			}

			void SRK_CALL write(const wchar_t* buf, size_t size);

			inline void SRK_CALL write(wchar_t c) {
				dilatation(1);
				_data[_pos++] = c;
			}

			inline void SRK_CALL unsafeWrite(wchar_t c) {
				_data[_pos++] = c;
			}

			void SRK_CALL dilatation(size_t size);

			inline std::wstring_view SRK_CALL toWString() {
				return _pos ? std::wstring_view(_data, _pos - 1) : std::wstring_view();
			}

		private:
			bool _needFree;
			size_t _pos;
			size_t _size;
			wchar_t* _data;
		};


		struct SRK_CORE_DLL DefaultFormater {
			template<typename Formater, typename T>
			bool SRK_CALL operator()(OutputBuffer& buf, Formater&& formater, T&& value) const {
				using namespace std::literals;

				using Type = std::remove_cvref_t<T>;

				if constexpr (NullPointer<Type>) {
					buf.write(L"nullptr"sv);
				} else if constexpr (StringData<Type>) {
					buf.write(value.data(), value.size());
				} else if constexpr (U8StringData<Type>) {
					buf.write(value);
				} else if constexpr (WStringData<Type>) {
					buf.write(value.data(), value.size());
				} else if constexpr (std::same_as<Type, bool>) {
					buf.write(value ? L"true"sv : L"false"sv);
				} else if constexpr (Arithmetic<Type>) {
					buf.write(std::to_wstring(value));
				} else if constexpr (std::is_enum_v<Type>) {
					buf.write(L"enum<"sv);
					buf.write(typeid(T).name());
					buf.write(L">("sv);
					buf.write(std::to_wstring((std::underlying_type_t<Type>)value));
					buf.write(L')');
				} else if constexpr (std::is_bounded_array_v<Type>) {
					constexpr auto n = std::extent_v<Type, 0>;
					buf.write(L"array<"sv);
					buf.write(typeid(value[0]).name());
					buf.write(L',');
					buf.write(std::to_wstring(n));
					buf.write(L">["sv);

					for (size_t i = 0; i < n; ++i) {
						Printer::print(buf, std::forward<Formater>(formater), value[i]);
						if (i + 1 < n) buf.write(L',');
					}

					buf.write(L']');
				} else if constexpr (std::convertible_to<Type, wchar_t const*>) {
					buf.write(value);
				} else if constexpr (std::convertible_to<Type, char const*>) {
					buf.write(value);
				} else if constexpr (std::convertible_to<Type, char8_t const*>) {
					buf.write(value);
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

					buf.write(L"pointer<"sv);
					buf.write(typeid(T).name());
					buf.dilatation(COUNT + 5);
					buf.unsafeWrite(L'>');
					buf.unsafeWrite(L'(');
					buf.unsafeWrite(L'0');
					buf.unsafeWrite(L'x');
					for (uint32_t i = 1; i <= COUNT; ++i) buf.unsafeWrite(MAP[values[COUNT - i]]);
					buf.unsafeWrite(L')');
				} else {
					buf.write(L"type<"sv);
					buf.write(typeid(T).name());
					buf.write(L'>');
				}

				return true;
			}
		};


		template<typename Formater, typename... Formaters>
		requires std::conjunction_v<std::is_default_constructible<Formaters>...>
		struct PriorityFormater {
			template<typename T>
			inline bool SRK_CALL operator()(OutputBuffer& buf, Formater&& formater, T&& value) const {
				if constexpr (sizeof...(Formaters) == 0) {
					return false;
				} else {
					return _execute<Formater, T, Formaters...>(buf, std::forward<Formater>(formater), std::forward<T>(value));
				}
			}

		private:
			template<typename Formater, typename T, typename Cur, typename... Others>
			inline bool SRK_CALL _execute(OutputBuffer& buf, Formater&& formater, T&& value) const {
				if constexpr (std::is_invocable_r_v<bool, Cur, OutputBuffer&, Formater&&, decltype(std::forward<T>(value))>) {
					Cur c;
					if (c(buf, std::forward<Formater>(formater), std::forward<T>(value))) return true;
				}
				
				if constexpr (sizeof...(Others) == 0) {
					return false;
				} else {
					return _execute<Formater, T, Others...>(buf, std::forward<Formater>(formater), std::forward<T>(value));
				}
			}
		};


		struct SRK_CORE_DLL DebuggerOutputer {
			bool SRK_CALL operator()(const std::wstring_view& data) const;
		};

		struct SRK_CORE_DLL ConsoleOutputer {
			inline bool SRK_CALL operator()(const std::wstring_view& data) const {
				std::wcout << data.data();
				return true;
			}
		};

		template<typename... Outputers>
		requires std::conjunction_v<std::is_invocable_r<bool, Outputers, const std::wstring_view&>...>&& std::conjunction_v<std::is_default_constructible<Outputers>...>
		struct PriorityOutputer {
			inline bool SRK_CALL operator()(const std::wstring_view& data) const {
				if constexpr (sizeof...(Outputers) == 0) {
					return false;
				} else {
					return _execute<Outputers...>(data);
				}
			}

		private:
			template<typename Cur, typename... Others>
			inline bool SRK_CALL _execute(const std::wstring_view& data) const {
				Cur c;
				if (c(data)) {
					return true;
				} else {
					if constexpr (sizeof...(Others) == 0) {
						return false;
					} else {
						return _execute<Others...>(data);
					}
				}
			}
		};

		template<typename Formater, typename Outputer, typename... Args>
		requires std::default_initializable<std::remove_cvref_t<Formater>> && std::invocable<Outputer, const std::wstring_view&> && std::default_initializable<std::remove_cvref_t<Outputer>>
		inline static void SRK_CALL print(Args&&... args) {
			Formater formater;
			Outputer outputer;
			print(formater, outputer, std::forward<Args>(args)...);
		}

		template<typename Formater, typename Outputer, typename... Args>
		requires std::invocable<Outputer, const std::wstring_view&>
		static void SRK_CALL print(Formater&& formater, Outputer&& outputer, Args&&... args) {
			const size_t MAX_LEN = 256;
			wchar_t wbuf[MAX_LEN];
			OutputBuffer buf(wbuf, MAX_LEN, false);

			((print(buf, std::forward<Formater>(formater), std::forward<Args>(args))), ...);

			buf.write(L'\0');

			std::scoped_lock lck(_mutex);
			outputer(buf.toWString());
		}

		template<typename Formater, typename T>
		static void SRK_CALL print(OutputBuffer& buf, Formater&& formater, T&& value) {
			if constexpr (std::is_invocable_r_v<bool, Formater, OutputBuffer&, Formater&&, decltype(std::forward<T>(value))>) {
				formater(buf, std::forward<Formater>(formater), std::forward<T>(value));
			}
		}

	private:
		inline static std::mutex _mutex;
	};

	template<typename Formater, typename Outputer, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>> && std::invocable<Outputer, const std::wstring_view&> && std::default_initializable<std::remove_cvref_t<Outputer>>
	inline void SRK_CALL print(Args&&... args) {
		Printer::print<Formater, Outputer>(std::forward<Args>(args)...);
	}

	template<typename Formater, typename Outputer, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>> && std::invocable<Outputer, const std::wstring_view&> && std::default_initializable<std::remove_cvref_t<Outputer>>
	inline void SRK_CALL println(Args&&... args) {
		using namespace std::literals;
		print<Formater, Outputer>(std::forward<Args>(args)..., L"\n"sv);
	}

	template<typename Formater = Printer::DefaultFormater, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>>
	inline void SRK_CALL printd(Args&&... args) {
		Printer::print<Formater, Printer::DebuggerOutputer, Args...>(std::forward<Args>(args)...);
	}

	template<typename Formater = Printer::DefaultFormater, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>>
	inline void SRK_CALL printdln(Args&&... args) {
		using namespace std::literals;
		printd<Formater>(std::forward<Args>(args)..., L"\n"sv);
	}

	template<typename Formater = Printer::DefaultFormater, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>>
	inline void SRK_CALL printc(Args&&... args) {
		Printer::print<Formater, Printer::ConsoleOutputer>(std::forward<Args>(args)...);
	}

	template<typename Formater = Printer::DefaultFormater, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>>
	inline void SRK_CALL printcln(Args&&... args) {
		using namespace std::literals;
		printc<Formater>(std::forward<Args>(args)..., L"\n"sv);
	}

	template<typename Formater = Printer::DefaultFormater, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>>
	inline void SRK_CALL printa(Args&&... args) {
		Printer::print<Formater, Printer::PriorityOutputer<Printer::DebuggerOutputer, Printer::ConsoleOutputer>>(std::forward<Args>(args)...);
	}

	template<typename Formater = Printer::DefaultFormater, typename... Args>
	requires std::default_initializable<std::remove_cvref_t<Formater>>
	inline void SRK_CALL printaln(Args&&... args) {
		using namespace std::literals;
		printa<Formater>(std::forward<Args>(args)..., L"\n"sv);
	}
}