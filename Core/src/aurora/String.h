#pragma once

#include "aurora/Global.h"

#if __has_include(<charconv>)
#	include <charconv>
#endif

#include <regex>

namespace aurora {
	class AE_CORE_DLL String {
	public:
		struct AE_CORE_DLL CharFlag {
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

		static bool AE_CALL isUTF8(const char* data, size_t len);

		//unicodeLen, utf8Len
		template<typename In>
		requires ConvertibleWStringData<std::remove_cvref_t<In>>
		static std::tuple<std::wstring::size_type, std::string::size_type> AE_CALL calcUnicodeToUtf8Length(In&& in) {
			if constexpr (WStringData<std::remove_cvref_t<In>>) {
				std::wstring::size_type s = 0;
				std::u8string::size_type d = 0;

				auto inSize = in.size();
				while (s < inSize) {
					if (uint_t<sizeof(wchar_t) * 8> c = in[s++]; c == 0) {
						break;
					} else if (c < 0x80) {  //
						//length = 1;
						++d;
					} else if (c < 0x800) {
						//length = 2;
						d += 2;
					} else if (c < 0x10000) {
						//length = 3;
						d += 3;
					} else if (c < 0x200000) {
						//length = 4;
						d += 4;
					}
				}

				return std::make_tuple(s, d);
			} else {
				return calcUnicodeToUtf8Length(std::wstring_view(std::forward<In>(in)));
			}
		}

		template<typename In, SameAnyOf<char, char8_t> Out>
		requires ConvertibleWStringData<std::remove_cvref_t<In>>
		static std::u8string::size_type AE_CALL UnicodeToUtf8(In&& in, Out* outBuffer, std::u8string::size_type outBufferSize) {
			if constexpr (WStringData<std::remove_cvref_t<In>>) {
				if (!outBuffer || !outBufferSize) return std::u8string::npos;
				if (in.empty()) {
					outBuffer[0] = 0;
					return 0;
				}

				auto [unicodeLen, utf8Len] = calcUnicodeToUtf8Length(in);
				if (outBufferSize < unicodeLen) return std::u8string::npos;

				return UnicodeToUtf8(in.data(), unicodeLen, outBuffer);
			} else {
				return UnicodeToUtf8(std::wstring_view(std::forward<In>(in)), outBuffer, outBufferSize);
			}
		}

		template<typename In, String8 Out = std::u8string>
		requires ConvertibleWStringData<std::remove_cvref_t<In>>
		static auto AE_CALL UnicodeToUtf8(In&& in) {
			if constexpr (WStringData<std::remove_cvref_t<In>>) {
				auto [unicodeLen, utf8Len] = calcUnicodeToUtf8Length(in);
				Out s;
				s.resize(utf8Len);
				UnicodeToUtf8(in.data(), unicodeLen, s.data());

				return std::move(s);
			} else {
				return UnicodeToUtf8(std::wstring_view(std::forward<In>(in)));
			}
		}

		//utf8Len, unicodeLen
		template<typename In>
		requires IsConvertibleString8Data<std::remove_cvref_t<In>>::value
		static std::tuple<std::u8string::size_type, std::wstring::size_type> AE_CALL calcUtf8ToUnicodeLength(In&& in) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				std::u8string::size_type s = 0;
				std::wstring::size_type d = 0;

				auto inSize = in.size();
				while (s < inSize) {
					if (uint8_t c = in[s]; c == 0) {
						break;
					} else if ((c & 0x80) == 0) {
						++s;
					} else if ((c & 0xE0) == 0xC0) {// 110x-xxxx 10xx-xxxx
						s += 2;
					} else if ((c & 0xF0) == 0xE0) {// 1110-xxxx 10xx-xxxx 10xx-xxxx
						s += 3;
					} else if ((c & 0xF8) == 0xF0) {// 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
						s += 4;
					} else {// 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
						s += 5;
					}
					++d;
				}

				return std::make_tuple(s, d);
			} else {
				return calcUtf8ToUnicodeLength(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			}
		}

		template<typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static std::wstring::size_type AE_CALL Utf8ToUnicode(In&& in, wchar_t* outBuffer, std::wstring::size_type outBufferSize) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				if (!outBuffer || !outBufferSize) return std::wstring::npos;
				if (in.empty()) {
					outBuffer[0] = 0;
					return 0;
				}

				auto [utf8Len, unicodeLen] = calcUtf8ToUnicodeLength(in);
				if (outBufferSize < unicodeLen) return std::wstring::npos;

				return Utf8ToUnicode(in, utf8Len, outBuffer);
			} else {
				return Utf8ToUnicode(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)), outBuffer, outBufferSize);
			}
		}

		template<SameAnyOf<char, char8_t> In>
		static std::wstring::size_type AE_CALL Utf8ToUnicode(const In* in, std::u8string::size_type inLen, wchar_t* out) {
			std::u8string::size_type s = 0;
			std::wstring::size_type d = 0;

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

		template<typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static std::wstring AE_CALL Utf8ToUnicode(In&& in) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				auto [utf8Len, unicodeLen] = calcUtf8ToUnicodeLength(in);
				std::wstring s;
				s.resize(unicodeLen);
				Utf8ToUnicode(in.data(), utf8Len, s.data());

				return std::move(s);
			} else {
				return Utf8ToUnicode(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			}
		}

		template<typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static std::wstring::size_type AE_CALL Utf8ToUnicode(In&& in, wchar_t*& out) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				if (in.empty()) {
					out = new wchar_t[1];
					out[0] = 0;
					return 0;
				}

				auto [utf8Len, unicodeLen] = calcUtf8ToUnicodeLength(in);
				++unicodeLen;
				out = new wchar_t[unicodeLen];
				auto len = Utf8ToUnicode(in.data(), utf8Len, out);
				out[len] = 0;

				return len;
			} else {
				return Utf8ToUnicode(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)), out);
			}
		}

		template<SameAnyOf<char, char8_t> Out>
		static std::u8string::size_type AE_CALL UnicodeToUtf8(const wchar_t* const in, std::wstring::size_type inLen, Out* out) {
			std::wstring::size_type s = 0;
			std::u8string::size_type d = 0;

			while (s < inLen) {
				if (wchar_t c = in[s++]; c < 0x80) {  //
					//length = 1;
					out[d++] = (char)c;
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
					out[d++] = 0xF0 | ((int)c >> 18);
					out[d++] = 0x80 | ((c >> 12) & 0x3F);
					out[d++] = 0x80 | ((c >> 6) & 0x3F);
					out[d++] = 0x80 | (c & 0x3F);
				}
			}

			return d;
		}

		template<typename In, typename Separator, std::invocable<const std::string_view&> Fn>
		requires ConvertibleStringData<std::remove_cvref_t<In>> && (std::derived_from<std::remove_cvref_t<Separator>, std::regex> || ConvertibleStringData<std::remove_cvref_t<Separator>>)
		static void AE_CALL split(In&& in, Separator&& separator, Fn&& fn) {
			if constexpr (std::derived_from<std::remove_cvref_t<Separator>, std::regex>) {
				if constexpr (StringData<std::remove_cvref_t<In>>) {
					std::regex_token_iterator itr(in.begin(), in.end(), separator, -1);
					std::regex_token_iterator<typename In::const_iterator> end;
					while (itr != end) {
						fn(std::string_view(&*itr->first, itr->length()));
						++itr;
					}
				} else {
					split(std::string_view(std::forward<In>(in)), std::forward<Separator>(separator), std::forward<Fn>(fn));
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
								fn(std::string_view(in.data() + begin, i - begin));
								i += step;
								begin = i;
							} else {
								++i;
							}
						} else {
							++i;
						}
					}

					fn(std::string_view(in.data() + begin, i - begin));
				} else {
					fn(in);
				}
			} else {
				split(std::string_view(std::forward<In>(in)), std::string_view(std::forward<Separator>(separator)), std::forward<Fn>(fn));
			}
		}

		template<typename In, std::invocable<const std::string_view&> Fn>
		requires ConvertibleStringData<std::remove_cvref_t<In>>
		static void AE_CALL split(In&& in, uint8_t flags, Fn&& fn) {
			if constexpr (StringData<std::remove_cvref_t<In>>) {
				size_t begin = 0, i = 0, size = in.size();
				while (i < size) {
					if (CHARS[in[i]] & flags) {
						fn(std::string_view(in.data() + begin, i - begin));
						++i;
						begin = i;
					} else {
						++i;
					}
				}

				fn(std::string_view(in.data() + begin, i - begin));
			} else {
				split(std::string_view(std::forward<In>(in)), flags, std::forward<Fn>(fn));
			}
		}

		template<typename In>
		requires ConvertibleString8Data<std::remove_cvref_t<In>>
		static ConvertToString8ViewType<std::remove_cvref_t<In>> AE_CALL trimQuotation(In&& in) {
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
		static ConvertToString8ViewType<std::remove_cvref_t<In>> AE_CALL trim(In&& in, uint8_t flags) {
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
		inline static std::string AE_CALL toString(T value, uint8_t base = 10) {
			char buf[sizeof(T) * 8 + 1];
#ifdef __cpp_lib_to_chars
			auto rst = std::to_chars(buf, buf + sizeof(buf), value, base);
			return std::move(std::string(buf, rst.ec == std::errc() ? rst.ptr - buf : 0));
#else
			if constexpr (std::same_as<T, int8_t>) {
				snprintf(buf, sizeof(buf), "%hhd", value);
			} else if constexpr (std::same_as<T, uint8_t>) {
				switch (base) {
				case 8:
					snprintf(buf, sizeof(buf), "%hho", value);
					break;
				case 16:
					snprintf(buf, sizeof(buf), "%hhx", value);
					break;
				default:
					snprintf(buf, sizeof(buf), "%hhu", value);
					break;
				}
			} else if constexpr (std::same_as<T, int16_t>) {
				snprintf(buf, sizeof(buf), "%hd", value);
			} else if constexpr (std::same_as<T, uint16_t>) {
				switch (base) {
				case 8:
					snprintf(buf, sizeof(buf), "%ho", value);
					break;
				case 16:
					snprintf(buf, sizeof(buf), "%hx", value);
					break;
				default:
					snprintf(buf, sizeof(buf), "%hu", value);
					break;
				}
			} else if constexpr (std::same_as<T, int32_t>) {
				snprintf(buf, sizeof(buf), "%d", value);
			} else if constexpr (std::same_as<T, uint32_t>) {
				switch (base) {
				case 8:
					snprintf(buf, sizeof(buf), "%o", value);
					break;
				case 16:
					snprintf(buf, sizeof(buf), "%x", value);
					break;
				default:
					snprintf(buf, sizeof(buf), "%u", value);
					break;
				}
			} else if constexpr (std::same_as<T, int64_t>) {
				if constexpr (sizeof(long) == sizeof(int64_t)) {
					snprintf(buf, sizeof(buf), "%ld", value);
				} else {
					snprintf(buf, sizeof(buf), "%lld", value);
				}
			} else if constexpr (std::same_as<T, uint64_t>) {
				switch (base) {
				case 8:
				{
					if constexpr (sizeof(long) == sizeof(uint64_t)) {
						snprintf(buf, sizeof(buf), "%lo", value);
					} else {
						snprintf(buf, sizeof(buf), "%llo", value);
					}

					break;
				}
				case 16:
				{
					if constexpr (sizeof(long) == sizeof(uint64_t)) {
						snprintf(buf, sizeof(buf), "%lx", value);
					} else {
						snprintf(buf, sizeof(buf), "%llx", value);
					}

					break;
				}
				default:
				{
					if constexpr (sizeof(long) == sizeof(uint64_t)) {
						snprintf(buf, sizeof(buf), "%lu", value);
					} else {
						snprintf(buf, sizeof(buf), "%llu", value);
					}

					break;
				}
				}
			}

			return std::move(std::string(buf));
#endif
		}

		template<std::floating_point T>
		inline static std::string AE_CALL toString(T value) {
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

		static std::string AE_CALL toString(const uint8_t* value, size_t size);

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
		inline static std::string::size_type AE_CALL find(In&& in, char c) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				auto p = (const std::remove_cvref_t<In>::value_type*)memchr(in.data(), c, in.size());
				return p ? p - in.data() : std::remove_cvref_t<In>::npos;
			} else {
				return find(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)), c);
			}
		}

		template<typename In, typename Compare>
		requires ConvertibleStringData<std::remove_cvref_t<In>> && ConvertibleStringData<std::remove_cvref_t<Compare>>
		inline static std::string::size_type AE_CALL find(In&& in, Compare&& compare) {
			if constexpr (StringData<std::remove_cvref_t<In>> && StringData<std::remove_cvref_t<Compare>>) {
				auto p = (const std::remove_cvref_t<In>::value_type*)memFind(in.data(), in.size(), compare.data(), compare.size());
				return p ? p - in.data() : std::remove_cvref_t<In>::npos;
			} else {
				return find(std::string_view(std::forward<In>(in)), std::string_view(std::forward<Compare>(compare)));
			}
		}

		template<typename In>
		requires ConvertibleStringData<std::remove_cvref_t<In>>
		inline static std::string::size_type AE_CALL find(In&& in, uint8_t flags) {
			if constexpr (String8Data<std::remove_cvref_t<In>>) {
				for (size_t i = 0, n = in.size(); i < n; ++i) {
					if (CHARS[in[i]] & flags) return i;
				}
				return std::remove_cvref_t<In>::npos;
			} else {
				return find(ConvertToString8ViewType<std::remove_cvref_t<In>>(std::forward<In>(in)));
			}
		}

		static bool AE_CALL isEqual(const char* str1, const char* str2);
	};
}