#pragma once

#include "srk/Core.h"
#include <array>
#include <filesystem>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

			inline void SRK_CALL write(const std::u8string& str) {
				write((const char*)str.data(), str.size());
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

			inline void SRK_CALL write(const char16_t* buf) {
				write(std::u16string_view(buf));
			}

			inline void SRK_CALL write(const std::u16string& str) {
				write(str.data(), str.size());
			}

			inline void SRK_CALL write(const std::u16string_view& str) {
				write(str.data(), str.size());
			}

			void SRK_CALL write(const char16_t* buf, size_t size);

			inline void SRK_CALL write(const char32_t* buf) {
				write(std::u32string_view(buf));
			}

			inline void SRK_CALL write(const std::u32string& str) {
				write(str.data(), str.size());
			}

			inline void SRK_CALL write(const std::u32string_view& str) {
				write(str.data(), str.size());
			}

			void SRK_CALL write(const char32_t* buf, size_t size);

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
		private:
			template<typename> struct IsStdArray : std::false_type {};
			template<typename T, size_t S> struct IsStdArray<std::array<T, S>> : std::true_type {};

			template<typename> struct IsStdVector : std::false_type {};
			template<typename T, typename A> struct IsStdVector<std::vector<T, A>> : std::true_type {};

			template<typename> struct IsStdList : std::false_type {};
			template<typename T, typename A> struct IsStdList<std::list<T, A>> : std::true_type {};

			template<typename Formater, typename T>
			void SRK_CALL _printArray(OutputBuffer& buf, Formater&& formater, T&& value, std::wstring_view typeStr, size_t n, const std::type_info& elementTypeInfo) const {
				using namespace std::string_view_literals;

				buf.write(typeStr);
				buf.write(L'<');
				buf.write(elementTypeInfo.name());
				buf.write(L',');
				buf.write(std::to_wstring(n));
				buf.write(L">["sv);

				size_t i = 0;
				for (const auto& itr : value) {
					Printer::print(buf, std::forward<Formater>(formater), itr);
					if (++i < n) buf.write(L',');
				}

				buf.write(L']');
			}

			template<typename> struct IsStdSet : std::false_type {};
			template<typename K, typename P, typename A> struct IsStdSet<std::set<K, P, A>> : std::true_type {};

			template<typename> struct IsStdUnorderedSet : std::false_type {};
			template<typename K, typename H, typename E> struct IsStdUnorderedSet<std::unordered_set<K, H, E>> : std::true_type {};

			template<typename Formater, typename T>
			void SRK_CALL _printSet(OutputBuffer& buf, Formater&& formater, T&& value, std::wstring_view typeStr, size_t n, const std::type_info& keyTypeInfo) const {
				using namespace std::string_view_literals;

				buf.write(typeStr);
				buf.write(L'<');
				buf.write(keyTypeInfo.name());;
				buf.write(L',');
				buf.write(std::to_wstring(n));
				buf.write(L">{"sv);

				size_t i = 0;
				for (const auto& itr : value) {
					Printer::print(buf, std::forward<Formater>(formater), itr);
					if (++i < n) buf.write(L',');
				}

				buf.write(L'}');
			}

			template<typename> struct IsStdMap : std::false_type {};
			template<typename K, typename V, typename P, typename A> struct IsStdMap<std::map<K, V, P, A>> : std::true_type {};

			template<typename> struct IsStdUnorderedMap : std::false_type {};
			template<typename K, typename V, typename H, typename E> struct IsStdUnorderedMap<std::unordered_map<K, V, H, E>> : std::true_type {};

			template<typename Formater, typename T>
			void SRK_CALL _printMap(OutputBuffer& buf, Formater&& formater, T&& value, std::wstring_view typeStr, size_t n, const std::type_info& keyTypeInfo, const std::type_info& valTypeInfo) const {
				using namespace std::string_view_literals;

				buf.write(typeStr);
				buf.write(L'<');
				buf.write(keyTypeInfo.name());
				buf.write(L':');
				buf.write(valTypeInfo.name());
				buf.write(L',');
				buf.write(std::to_wstring(n));
				buf.write(L">{"sv);

				size_t i = 0;
				for (const auto& itr : value) {
					Printer::print(buf, std::forward<Formater>(formater), itr.first);
					buf.write(L':');
					Printer::print(buf, std::forward<Formater>(formater), itr.second);
					if (++i < n) buf.write(L',');
				}

				buf.write(L'}');
			}

		public:
			template<typename Formater, typename T>
			bool SRK_CALL operator()(OutputBuffer& buf, Formater&& formater, T&& value) const {
				using namespace std::string_view_literals;

				using Type = std::remove_cvref_t<T>;

				if constexpr (NullPointer<Type>) {
					buf.write(L"nullptr"sv);
				} else if constexpr (String8Data<Type> || WStringData<Type> || String16Data<Type> || String32Data<Type>) {
					buf.write(value);
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
				}  else if constexpr (std::is_same_v<Type, std::filesystem::path>) {
					buf.write(value.wstring());
				} else if constexpr (std::is_bounded_array_v<Type>) {
					_printArray(buf, std::forward<Formater>(formater), std::forward<T>(value), L"array"sv, std::extent_v<Type, 0>, typeid(value[0]));
				} else if constexpr (IsStdArray<Type>::value) {
					_printArray(buf, std::forward<Formater>(formater), std::forward<T>(value), L"std_array"sv, value.size(), typeid(Type::value_type));
				} else if constexpr (IsStdVector<Type>::value) {
					_printArray(buf, std::forward<Formater>(formater), std::forward<T>(value), L"std_vector"sv, value.size(), typeid(Type::value_type));
				} else if constexpr (IsStdList<Type>::value) {
					_printArray(buf, std::forward<Formater>(formater), std::forward<T>(value), L"std_list"sv, value.size(), typeid(Type::value_type));
				} else if constexpr (IsStdSet<Type>::value) {
					_printSet(buf, std::forward<Formater>(formater), std::forward<T>(value), L"std_set"sv, value.size(), typeid(Type::key_type));
				} else if constexpr (IsStdUnorderedSet<Type>::value) {
					_printSet(buf, std::forward<Formater>(formater), std::forward<T>(value), L"std_unordered_set"sv, value.size(), typeid(Type::key_type));
				} else if constexpr (IsStdMap<Type>::value) {
					_printMap(buf, std::forward<Formater>(formater), std::forward<T>(value), L"std_map"sv, value.size(), typeid(Type::key_type), typeid(Type::value_type::second_type));
				} else if constexpr (IsStdUnorderedMap<Type>::value) {
					_printMap(buf, std::forward<Formater>(formater), std::forward<T>(value), L"std_unordered_map"sv, value.size(), typeid(Type::key_type), typeid(Type::value_type::second_type));
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


		template<typename... Formaters>
		requires std::conjunction_v<std::is_default_constructible<Formaters>...>
		struct PriorityFormater {
			template<typename Formater, typename T>
			inline bool SRK_CALL operator()(OutputBuffer& buf, Formater&& formater, T&& value) const {
				if constexpr (sizeof...(Formaters) == 0) {
					return false;
				} else {
					return _execute<Formater, T, Formaters...>(buf, std::forward<Formater>(formater), std::forward<T>(value));
				}
			}

		private:
			template<typename Formater, typename T, typename CurFormater, typename... OtherFormaters>
			inline bool SRK_CALL _execute(OutputBuffer& buf, Formater&& formater, T&& value) const {
				if constexpr (std::is_invocable_r_v<bool, CurFormater, OutputBuffer&, Formater&&, decltype(std::forward<T>(value))>) {
					CurFormater f;
					if (f(buf, std::forward<Formater>(formater), std::forward<T>(value))) return true;
				}
				
				if constexpr (sizeof...(OtherFormaters) == 0) {
					return false;
				} else {
					return _execute<Formater, T, OtherFormaters...>(buf, std::forward<Formater>(formater), std::forward<T>(value));
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
		using namespace std::string_view_literals;
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
		using namespace std::string_view_literals;
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
		using namespace std::string_view_literals;
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
		using namespace std::string_view_literals;
		printa<Formater>(std::forward<Args>(args)..., L"\n"sv);
	}
}