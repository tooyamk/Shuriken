#pragma once

#include "srk/Memory.h"
#include "srk/Concepts.h"

#if __has_include(<charconv>)
#	include <charconv>
#endif

#include <regex>

namespace srk {
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


	class SRK_CORE_DLL StringUtility {
	public:
		StringUtility() = delete;

		struct SRK_CORE_DLL CharFlag {
			inline static constexpr uint8_t WHITE_SPACE = 0b1;
			inline static constexpr uint8_t NEW_LINE = 0b10;
		};


		inline static constexpr uint8_t CHARS[] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, CharFlag::WHITE_SPACE,  //  0-9 9=\t
			CharFlag::WHITE_SPACE | CharFlag::NEW_LINE, CharFlag::WHITE_SPACE, CharFlag::WHITE_SPACE, CharFlag::WHITE_SPACE | CharFlag::NEW_LINE, 0, 0, 0, 0, 0, 0,  // 10- 19 10=\n, 11=\v, 12=\f, 13=\r
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 20- 29
			0, 0, CharFlag::WHITE_SPACE, 0, 0, 0, 0, 0, 0, 0,  // 30- 39 32=space
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 40- 49
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 50- 59
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 60- 69
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 70- 79
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 80- 89
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 90- 99
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //100-109
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //110-119
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //120-129
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //130-139
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //140-149
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //150-159
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //160-169
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //170-179
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //180-189
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //190-199
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //200-209
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //210-219
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //220-229
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //230-239
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //240-249
			0, 0, 0, 0, 0, 0			   //250-255
		};

		static bool SRK_CALL isUtf8(const char* data, size_t len);

		//u8chars, u8bytes
		template<typename In>
		requires IsConvertibleString8Data<std::remove_cvref_t<In>>::value || ConvertibleAnyWideStringData<std::remove_cvref_t<In>>
		static std::tuple<size_t, size_t> SRK_CALL calcUtf8(In&& in) {
			if constexpr (IsConvertibleString8Data<std::remove_cvref_t<In>>::value) {
				if constexpr (ConvertibleAnyOf<std::remove_cvref_t<In>, char const*, char8_t const*>) {
					return calcUtf8(ConvertToAnyStringViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
				} else {
					size_t chars = 0;
					size_t bytes = 0;

					auto inSize = in.size();
					while (bytes < inSize) {
						uint8_t c = in[bytes];
						if (c == 0)  break;

						if ((c & 0x80) == 0) {
							++bytes;
						} else if ((c & 0xE0) == 0xC0) {// 110x-xxxx 10xx-xxxx
							bytes += 2;
						} else if ((c & 0xF0) == 0xE0) {// 1110-xxxx 10xx-xxxx 10xx-xxxx
							bytes += 3;
						} else if ((c & 0xF8) == 0xF0) {// 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
							bytes += 4;
						} else {// 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
							bytes += 5;
						}
						++chars;
					}

					return std::make_tuple(chars, bytes);
				}
			} else {
				if constexpr (ConvertibleAnyOf<std::remove_cvref_t<In>, wchar_t const*, char16_t const*, char32_t const*>) {
					return calcUtf8(ConvertToAnyStringViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
				} else {
					size_t chars = 0;
					size_t bytes = 0;

					auto inSize = in.size();
					while (chars < inSize) {
						auto c = in[chars];
						if (c == 0) break;
						++chars;

						if (c < 0x80) {
							//length = 1;
							++bytes;
						} else if (c < 0x800) {
							//length = 2;
							bytes += 2;
						} else if (c < 0x10000) {
							//length = 3;
							bytes += 3;
						} else if (c < 0x200000) {
							//length = 4;
							bytes += 4;
						}
					}

					return std::make_tuple(chars, bytes);
				}
			}
		}

		template<SameAnyOf<char, char8_t> Out, typename In>
		requires ConvertibleAnyWideStringData<std::remove_cvref_t<In>>
		static size_t SRK_CALL wideToUtf8(In&& in, Out* outBuffer, size_t outBufferSize) {
			if constexpr (ConvertibleAnyOf<std::remove_cvref_t<In>, wchar_t const*, char16_t const*, char32_t const*>) {
				return wideToUtf8(ConvertToAnyStringViewType<std::remove_cvref_t<In>>(std::forward<In>(in)), outBuffer, outBufferSize);
			} else {
				if (!outBuffer || !outBufferSize) return -1;
				if (in.empty()) {
					outBuffer[0] = 0;
					return 0;
				}

				auto [chars, bytes] = calcUtf8(in);
				if (outBufferSize < bytes) return -1;

				return wideToUtf8(in.data(), chars, outBuffer);
			}
		}

		template<String8 Out, typename In>
		requires ConvertibleAnyWideStringData<std::remove_cvref_t<In>>
		static auto SRK_CALL wideToUtf8(In&& in) {
			if constexpr (ConvertibleAnyOf<std::remove_cvref_t<In>, wchar_t const*, char16_t const*, char32_t const*>) {
				return wideToUtf8<Out>(ConvertToAnyStringViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			} else {
				auto [chars, bytes] = calcUtf8(in);
				Out s;
				s.resize(bytes);
				wideToUtf8(in.data(), chars, s.data());

				return std::move(s);
			}
		}

		template<SameAnyOf<char, char8_t> Out, SameAnyOf<wchar_t, char16_t, char32_t> In>
		static size_t SRK_CALL wideToUtf8(const In* const in, size_t inChars, Out* out) {
			size_t s = 0;
			size_t d = 0;

			while (s < inChars) {
				if (auto c = in[s++]; c < 0x80) {
					//length = 1;
					out[d++] = (Out)c;
				} else if (c < 0x800) {
					//length = 2;
					out[d++] = 0xC0 | (c >> 6);
					out[d++] = 0x80 | (c & 0x3F);
				} else if (c < 0x10000) {
					//length = 3;
					out[d++] = 0xE0 | (c >> 12);
					out[d++] = 0x80 | ((c >> 6) & 0x3F);
					out[d++] = 0x80 | (c & 0x3F);
				} else if (c < 0x200000) {
					//length = 4;
					out[d++] = 0xF0 | ((int32_t)c >> 18);
					out[d++] = 0x80 | ((c >> 12) & 0x3F);
					out[d++] = 0x80 | ((c >> 6) & 0x3F);
					out[d++] = 0x80 | (c & 0x3F);
				}
			}

			return d;
		}

		template<SameAnyOf<wchar_t, char16_t, char32_t> Out, typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static size_t SRK_CALL utf8ToWide(In&& in, Out* outBuffer, size_t outBufferSize) {
			if constexpr (ConvertibleAnyOf<std::remove_cvref_t<In>, char const*, char8_t const*>) {
				return utf8ToWide(ConvertToAnyStringViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			} else {
				if (!outBuffer || !outBufferSize) return -1;
				if (in.empty()) {
					outBuffer[0] = 0;
					return 0;
				}

				auto [chars, bytes] = calcUtf8(in);
				if (outBufferSize < chars) return -1;

				return utf8ToWide(in, bytes, outBuffer);
			}
		}

		template<SameAnyOf<wchar_t, char16_t, char32_t> Out, SameAnyOf<char, char8_t> In>
		static size_t SRK_CALL utf8ToWide(const In* in, size_t inLen, Out* out) {
			size_t s = 0;
			size_t d = 0;

			while (s < inLen) {
				if (uint8_t c = in[s]; (c & 0x80) == 0) {
					out[d++] = in[s++];
				} else if ((c & 0xE0) == 0xC0) {// 110x-xxxx 10xx-xxxx
					out[d++] = ((c & 0x3F) << 6) | (in[s + 1] & 0x3F);
					s += 2;
				} else if ((c & 0xF0) == 0xE0) {// 1110-xxxx 10xx-xxxx 10xx-xxxx
					out[d++] = ((c & 0x1F) << 12) | ((in[s + 1] & 0x3F) << 6) | (in[s + 2] & 0x3F);
					s += 3;
				} else if ((c & 0xF8) == 0xF0) {// 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
					out[d++] = ((in[s + 1] & 0x3F) << 12) | ((in[s + 2] & 0x3F) << 6) | (in[s + 3] & 0x3F);
					/*
					//wideChar = (in[s + 0] & 0x0F) << 18;
					wideChar = (in[s + 1] & 0x3F) << 12;
					wideChar |= (in[s + 2] & 0x3F) << 6;
					wideChar |= (in[s + 3] & 0x3F);
					*/
					s += 4;
				} else {// 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
					out[d++] = ((in[s + 2] & 0x3F) << 12) | ((in[s + 3] & 0x3F) << 6) | (in[s + 4] & 0x3F);
					/*
					//wideChar = (in[s + 0] & 0x07) << 24;
					//wideChar = (in[s + 1] & 0x3F) << 18;
					wideChar = (in[s + 2] & 0x3F) << 12;
					wideChar |= (in[s + 3] & 0x3F) << 6;
					wideChar |= (in[s + 4] & 0x3F);
					*/
					s += 5;
				}
			}

			return d;
		}

		template<SameAnyOf<std::wstring, std::u16string, std::u32string> Out, typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static Out SRK_CALL utf8ToWide(In&& in) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				auto [chars, bytes] = calcUtf8(in);
				Out s;
				s.resize(chars);
				utf8ToWide(in.data(), bytes, s.data());

				return std::move(s);
			} else {
				return utf8ToWide<Out>(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			}
		}

		template<SameAnyOf<wchar_t, char16_t, char32_t> Out, typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static size_t SRK_CALL utf8ToWide(In&& in, Out*& out) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				if (in.empty()) {
					out = new wchar_t[1];
					out[0] = 0;
					return 0;
				}

				auto [chars, bytes] = calcUtf8(in);
				out = new Out[chars + 1];
				auto len = utf8ToWide(in.data(), bytes, out);
				out[len] = 0;

				return len;
			} else {
				return utf8ToWide(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)), out);
			}
		}

		template<typename In, typename Separator, typename Fn, typename... Args>
		requires ConvertibleStringData<std::remove_cvref_t<In>> && InvocableAnyOfResult<std::remove_cvref_t<Fn>, std::tuple<void, bool>, const std::string_view&, Args...> && (std::derived_from<std::remove_cvref_t<Separator>, std::regex> || ConvertibleStringData<std::remove_cvref_t<Separator>>)
		static void SRK_CALL split(In&& in, Separator&& separator, Fn&& fn, Args&&... args) {
			if constexpr (std::derived_from<std::remove_cvref_t<Separator>, std::regex>) {
				if constexpr (StringData<std::remove_cvref_t<In>>) {
					std::regex_token_iterator itr(in.begin(), in.end(), separator, -1);
					std::regex_token_iterator<typename In::const_iterator> end;
					while (itr != end) {
						if constexpr (std::same_as<std::invoke_result_t<Fn, const std::string_view&, Args...>, void>) {
							fn(std::string_view(&*itr->first, itr->length()), std::forward<Args>(args)...);
						} else {
							if (!fn(std::string_view(&*itr->first, itr->length()), std::forward<Args>(args)...)) return;
						}
						++itr;
					}
				} else {
					split(std::string_view(std::forward<In>(in)), std::forward<Separator>(separator), std::forward<Fn>(fn), std::forward<Args>(args)...);
				}
			} else if constexpr (StringData<std::remove_cvref_t<In>> && StringData<std::remove_cvref_t<Separator>>) {
				if (auto step = separator.size(); step) {
					size_t begin = 0, i = 0, size = in.size();
					while (i < size) {
						if (in[i] == separator[0]) {
							auto found = true;
							for (size_t j = 1; j < step; ++j) {
								if (in[i + j] != separator[j]) {
									found = false;
									break;
								}
							}

							if (found) {
								if constexpr (std::same_as<std::invoke_result_t<Fn, const std::string_view&, Args...>, void>) {
									fn(std::string_view(in.data() + begin, i - begin), std::forward<Args>(args)...);
								} else {
									if (!fn(std::string_view(in.data() + begin, i - begin), std::forward<Args>(args)...)) return;
								}
								i += step;
								begin = i;
							} else {
								++i;
							}
						} else {
							++i;
						}
					}

					fn(std::string_view(in.data() + begin, i - begin), std::forward<Args>(args)...);
				} else {
					fn(in, std::forward<Args>(args)...);
				}
			} else {
				split(std::string_view(std::forward<In>(in)), std::string_view(std::forward<Separator>(separator)), std::forward<Fn>(fn), std::forward<Args>(args)...);
			}
		}

		template<typename In, typename Fn, typename... Args>
		requires ConvertibleStringData<std::remove_cvref_t<In>> && InvocableAnyOfResult<std::remove_cvref_t<Fn>, std::tuple<void, bool>, const std::string_view&, Args...>
		static void SRK_CALL split(In&& in, uint8_t flags, Fn&& fn, Args&&... args) {
			if constexpr (StringData<std::remove_cvref_t<In>>) {
				size_t begin = 0, i = 0, size = in.size();
				while (i < size) {
					if (CHARS[in[i]] & flags) {
						if constexpr (std::same_as<std::invoke_result_t<Fn, const std::string_view&, Args...>, void>) {
							fn(std::string_view(in.data() + begin, i - begin), std::forward<Args>(args)...);
						} else {
							if (!fn(std::string_view(in.data() + begin, i - begin), std::forward<Args>(args)...)) return;
						}
						++i;
						begin = i;
					} else {
						++i;
					}
				}

				fn(std::string_view(in.data() + begin, i - begin), std::forward<Args>(args)...);
			} else {
				split(std::string_view(std::forward<In>(in)), flags, std::forward<Fn>(fn), std::forward<Args>(args)...);
			}
		}

		template<typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static ConvertToString8ViewType<std::remove_cvref_t<In>> SRK_CALL trimQuotation(In&& in) {
			if constexpr (StringData<std::remove_cvref_t<In>>) {
				auto size = in.size();

				if (size >= 2) {
					if (in[0] == '\"' && in[size - 1] == '\"') {
						return ConvertToString8ViewType<std::remove_cvref_t<In>>(in.data() + 1, size - 2);
					} else {
						return in;
					}
				} else if (size == 1 && in[0] == '\"') {
					return ConvertToString8ViewType<std::remove_cvref_t<In>>();
				} else {
					return in;
				}
			} else {
				return trimQuotation(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			}
		}

		template<typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static ConvertToString8ViewType<std::remove_cvref_t<In>> SRK_CALL trim(In&& in, uint8_t flags) {
			if constexpr (StringData<std::remove_cvref_t<In>>) {
				auto size = in.size();

				if (size) {
					size_t left = 0, right = size - 1;
					do {
						if (CHARS[in[left]] & flags) {
							++left;
						} else {
							break;
						}
					} while (left < size);

					if (left == size) return ConvertToString8ViewType<std::remove_cvref_t<In>>();

					while (right > left) {
						if (CHARS[in[right]] & flags) {
							--right;
						} else {
							break;
						}
					}

					return ConvertToString8ViewType<std::remove_cvref_t<In>>(in.data() + left, right - left + 1);
				} else {
					return in;
				}
			} else {
				return trim(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			}
		}

		template<std::integral T>
		inline static std::string SRK_CALL toString(T value, uint8_t base = 10) {
			char buf[sizeof(T) * 8 + 1];
			auto n = toString(buf, sizeof(buf), value, base);
			return std::move(n == std::string::npos ? std::string() : std::string(buf, n));
		}

		template<std::integral T>
		inline static std::string::size_type SRK_CALL toString(char* dst, size_t dstSize, T value, uint8_t base = 10) {
#ifdef __cpp_lib_to_chars
			auto rst = std::to_chars(dst, dst + dstSize, value, base);
			return rst.ec == std::errc() ? rst.ptr - dst : std::string::npos;
#else
			if constexpr (std::same_as<T, int8_t>) {
				return snprintf(dst, dstSize, "%hhd", value);
			} else if constexpr (std::same_as<T, uint8_t>) {
				switch (base) {
				case 8:
					return snprintf(dst, dstSize, "%hho", value);
				case 16:
					return snprintf(dst, dstSize, "%hhx", value);
				default:
					return snprintf(dst, dstSize, "%hhu", value);
				}
			} else if constexpr (std::same_as<T, int16_t>) {
				return snprintf(dst, dstSize, "%hd", value);
			} else if constexpr (std::same_as<T, uint16_t>) {
				switch (base) {
				case 8:
					return snprintf(dst, dstSize, "%ho", value);
				case 16:
					return snprintf(dst, dstSize, "%hx", value);
				default:
					return snprintf(dst, dstSize, "%hu", value);
				}
			} else if constexpr (std::same_as<T, int32_t>) {
				return snprintf(dst, dstSize, "%d", value);
			} else if constexpr (std::same_as<T, uint32_t>) {
				switch (base) {
				case 8:
					return snprintf(dst, dstSize, "%o", value);
				case 16:
					return snprintf(dst, dstSize, "%x", value);
				default:
					return snprintf(dst, dstSize, "%u", value);
				}
			} else if constexpr (std::same_as<T, int64_t>) {
				if constexpr (sizeof(long) == sizeof(int64_t)) {
					return snprintf(dst, dstSize, "%ld", value);
				} else {
					return snprintf(dst, dstSize, "%lld", value);
				}
			} else if constexpr (std::same_as<T, uint64_t>) {
				switch (base) {
				case 8:
				{
					if constexpr (sizeof(long) == sizeof(uint64_t)) {
						return snprintf(dst, dstSize, "%lo", value);
					} else {
						return snprintf(dst, dstSize, "%llo", value);
					}
				}
				case 16:
				{
					if constexpr (sizeof(long) == sizeof(uint64_t)) {
						return snprintf(dst, dstSize, "%lx", value);
					} else {
						return snprintf(dst, dstSize, "%llx", value);
					}
				}
				default:
				{
					if constexpr (sizeof(long) == sizeof(uint64_t)) {
						return snprintf(dst, dstSize, "%lu", value);
					} else {
						return snprintf(dst, dstSize, "%llu", value);
					}
				}
				}
			}
#endif
		}

		template<std::floating_point T>
		inline static std::string SRK_CALL toString(T value) {
			char buf[33];
#ifdef __cpp_lib_to_chars
			auto rst = std::to_chars(buf, buf + sizeof(buf), value);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
#else
			if constexpr (std::same_as<T, float32_t>) {
				snprintf(buf, sizeof(buf), "%f", value);
			} else {
				snprintf(buf, sizeof(buf), "%lf", value);
			}

			return std::move(std::string(buf));
#endif
		}

		static std::string SRK_CALL toString(const uint8_t* value, size_t size);

		template<std::integral Out, typename In>
		requires ConvertibleStringData<std::remove_cvref_t<In>>
		inline static Out toNumber(In&& in, int32_t base = 10) {
			if constexpr (StringData<std::remove_cvref_t<In>>) {
				Out value;
#ifdef __cpp_lib_to_chars
				auto begin = in.data();
				return std::from_chars(begin, begin + in.size(), value, base).ec == std::errc() ? value : 0;
#else
				if constexpr (std::unsigned_integral<Out>) {
					if constexpr (sizeof(Out) <= sizeof(uint32_t)) {
						return std::stoul(in.data(), nullptr, base);
					} else {
						return std::stoull(in.data(), nullptr, base);
					}
				} else {
					if constexpr (sizeof(Out) <= sizeof(int32_t)) {
						return std::stol(in.data(), nullptr, base);
					} else {
						return std::stoll(in.data(), nullptr, base);
					}
				}
#endif
			} else {
				return toNumber<Out>(std::string_view(std::forward<In>(in)), base);
			}
		}

		template<std::floating_point Out, typename In>
		requires ConvertibleStringData<std::remove_cvref_t<In>>
		inline static Out toNumber(const In& in) {
			if constexpr (StringData<std::remove_cvref_t<In>>) {
				Out value;
#ifdef __cpp_lib_to_chars
				auto begin = in.data();
				return std::from_chars(begin, begin + in.size(), value).ec == std::errc() ? value : 0.;
#else
				if constexpr (std::same_as<Out, float32_t>) {
					return std::stof(in.data(), nullptr);
				} else {
					return std::stod(in.data(), nullptr);
				}
#endif
			} else {
				return toNumber<Out>(std::string_view(std::forward<In>(in)));
			}
		}

		template<typename In>
		requires ConvertibleStringData<std::remove_cvref_t<In>>
		inline static std::string::size_type SRK_CALL find(In&& in, char c) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				using T = typename std::remove_cvref_t<In>::value_type;
				auto p = (const T*)memchr(in.data(), c, in.size());
				return p ? p - in.data() : std::remove_cvref_t<In>::npos;
			} else {
				return find(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)), c);
			}
		}

		template<typename In, typename Compare>
		requires ConvertibleStringData<std::remove_cvref_t<In>>&& ConvertibleStringData<std::remove_cvref_t<Compare>>
		inline static std::string::size_type SRK_CALL find(In&& in, Compare&& compare) {
			if constexpr (StringData<std::remove_cvref_t<In>> && StringData<std::remove_cvref_t<Compare>>) {
				using T = typename std::remove_cvref_t<In>::value_type;
				auto p = (const T*)Memory::find(in.data(), in.size(), compare.data(), compare.size());
				return p ? p - in.data() : std::remove_cvref_t<In>::npos;
			} else {
				return find(std::string_view(std::forward<In>(in)), std::string_view(std::forward<Compare>(compare)));
			}
		}

		template<typename In>
		requires ConvertibleStringData<std::remove_cvref_t<In>>
		inline static std::string::size_type SRK_CALL find(In&& in, uint8_t flags) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				for (size_t i = 0, n = in.size(); i < n; ++i) {
					if (CHARS[in[i]] & flags) return i;
				}
				return std::remove_cvref_t<In>::npos;
			} else {
				return find(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			}
		}

		static bool SRK_CALL equal(const char* str1, const char* str2);
	};
}