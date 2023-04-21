#include "srk/Core.h"
#include <optional>

namespace srk {
	struct bad_type {};


	template<template<typename...> typename Base, typename Derived>
	struct is_base_of_template {
	private:
		struct _impl {
			template<typename... Args> static constexpr std::true_type test(const Base<Args...>*);
			static constexpr std::false_type test(...);
			using type = decltype(test(std::declval<Derived*>()));
		};

	public:
		static constexpr bool value = _impl::type::value;
	};
	template<typename Derived, template<typename...> typename Base> concept derived_from_template = is_base_of_template<Base, Derived>::value;


	template<typename... Types>
	struct type_array {
	private:
		template<bool B, size_t I, size_t N, typename PrevType, typename... OtherTypes>
		struct _at;

		template<bool B, size_t I, size_t N>
		struct _at<B, I, N, bad_type> {
			using type = bad_type;
		};

		template<size_t I, size_t N, typename PrevType, typename... OtherTypes>
		struct _at<false, I, N, PrevType, OtherTypes...> {
			using type = std::conditional_t<I - 1 == N, PrevType, bad_type>;
		};

		template<size_t I, size_t N, typename PrevType, typename CurType, typename... OtherTypes>
		struct _at<true, I, N, PrevType, CurType, OtherTypes...> : _at<(I < N) && (N < sizeof...(Types)), I + 1, N, CurType, OtherTypes...> {};

		template<bool B, size_t I, typename T, typename PrevType, typename... OtherTypes>
		struct _find;

		template<bool B, size_t I, typename T>
		struct _find<B, I, T, bad_type> {
			inline static constexpr std::optional<size_t> index = std::nullopt;
		};

		template<size_t I, typename T, typename PrevType, typename... OtherTypes>
		struct _find<false, I, T, PrevType, OtherTypes...> {
			inline static constexpr std::optional<size_t> index = std::same_as<T, PrevType> ? std::make_optional<size_t>(I - 1) : std::nullopt;
		};

		template<size_t I, typename T, typename PrevType, typename CurType, typename... OtherTypes>
		struct _find<true, I, T, PrevType, CurType, OtherTypes...> : _find<!std::same_as<T, CurType> && (I + 1 < sizeof...(Types)), I + 1, T, CurType, OtherTypes...> {};

	public:
		inline static constexpr size_t size = sizeof...(Types);

		template<size_t I>
		using at = typename _at<true, 0, I, bad_type, Types...>::type;

		template<typename T>
		inline static constexpr std::optional<size_t> find = _find<true, 0, T, bad_type, Types...>::index;

		template<typename T>
		inline static constexpr bool has = find<T> != std::nullopt;
	};

	//template<typename T, typename... Types> using convert_type = std::enable_if_t<type_array<Types...>::template has<T>, std::conditional_t<(type_array<Types...>::template find<T>.value() & 0b1) == 0b1, typename type_array<Types...>::template element<type_array<Types...>::template find<T>.value() - 1>, typename type_array<Types...>::template element<type_array<Types...>::template find<T>.value() + 1>>>;

	namespace statical {
		struct bad_element {};

		template<auto Val, std::convertible_to<decltype(Val)> T = decltype(Val)>
		struct nontype_value {
			using type = T;
			static constexpr T value = Val;
		};

		template<size_t I, typename T>
		struct index_element {
			using type = T;
			static constexpr size_t index = I;
		};

		template<typename F, typename S>
		struct pair_element {
			using first = F;
			using second = S;
		};

		template<typename T>
		struct is_base_of_nontype_value {
		private:
			template<typename T>
			struct _impl {
				template<auto Val, typename... Args> static constexpr std::true_type test(const nontype_value<Val, Args...>*);
				static constexpr std::false_type test(...);
				using type = decltype(test(std::declval<T*>()));
			};

		public:
			static constexpr bool value = _impl<T>::type::value;
		};

		template<typename T>
		concept derived_from_nontype_value = is_base_of_nontype_value<T>::value;

		//requires requires { typename std::enable_if_t<std::conjunction_v<is_base_of_template<type_value, Values>...>>; }

		/*
		template<typename... Values>
		struct array;

		template<>
		struct array<> {
			static constexpr size_t size = 0;

			template<size_t I>
			using at = bad_element;

			template<size_t I, typename T>
			using set = array<T>;

			template<typename T, bool Convertible = false>
			using find_first = bad_element;

			template<typename T, bool Convertible = false>
			using find_last = bad_element;

			template<typename T, bool Convertible = false>
			using has = std::false_type;

			template<typename... Args>
			using push_back = array<Args...>;

			template<size_t I, typename T>
			using insert = array<T>;

			template<typename Arr>
			requires is_base_of_template_v<array, Arr>
			//template<template<typename...> typename Arr>
			using concat = Arr;

			template<size_t I>
			using erase = array<>;

			using pop_back = array<>;
		};
		*/

		template<typename... Values>
		struct array {
		private:
			template<typename T1, typename T2, bool Convertible>
			static constexpr bool _is_equal() {
				if constexpr (derived_from_nontype_value<T1>) {
					if constexpr (derived_from_nontype_value<T2>) {
						if constexpr (Convertible) {
							if constexpr (!(std::convertible_to<T1::type, T2::type> || std::convertible_to<T2::type, T1::type>)) {
								return false;
							}
						} else {
							if constexpr (!std::same_as<T1, T2>) {
								return false;
							}
						}

						return T1::value == T2::value;
					} else {
						return false;
					}
				} else {
					if constexpr (derived_from_nontype_value<T2>) {
						return false;
					} else {
						return Convertible ? (std::convertible_to<T1, T2> || std::convertible_to<T2, T1>) : std::is_same_v<T1, T2>;
					}
				}
			}

			template<size_t CurIdx, size_t DstIdx, typename CurType, typename... OtherTypes>
			static constexpr auto _at() {
				if constexpr (CurIdx < DstIdx) {
					return _at<CurIdx + 1, DstIdx, OtherTypes...>();
				} else if constexpr (CurIdx == DstIdx) {
					return index_element<CurIdx, CurType>();
				} else {
					return bad_element();
				}
			}

			template<size_t DstIdx>
			static constexpr auto _at() {
				if constexpr (DstIdx < sizeof...(Values)) {
					return _at<0, DstIdx, Values...>();
				} else {
					return bad_element();
				}
			}

			template<size_t CurIdx, typename Arr, size_t DstIdx, typename T, typename CurType, typename... OtherTypes>
			static constexpr auto _set() {
				using arr = Arr::template push_back<std::conditional_t<CurIdx == DstIdx, T, CurType>>;

				if constexpr (CurIdx + 1 == sizeof...(Values)) {
					return arr();
				} else {
					return _set<CurIdx + 1, arr, DstIdx, T, OtherTypes...>();
				}
			}

			template<size_t DstIdx, typename T>
			static constexpr auto _set() {
				if constexpr (DstIdx < sizeof...(Values)) {
					return _set<0, array<>, DstIdx, T, Values...>();
				} else {
					return array<Values..., T>();
				}
			}

			template<typename T, bool Convertible, size_t I>
			static constexpr auto _find_first() {
				if constexpr (I < sizeof...(Values)) {
					using element = at<I>;

					if constexpr (_is_equal<T, element::type, Convertible>()) {
						return index_element<I, element::type>();
					} else if constexpr (I + 1 < sizeof...(Values)) {
						return _find_first<T, Convertible, I + 1>();
					} else {
						return bad_element();
					}
				} else {
					return bad_element();
				}
			}

			template<typename T, bool Convertible, size_t I>
			static constexpr auto _find_last() {
				if constexpr (I > 0) {
					using element = at<I - 1>;

					if constexpr (_is_equal<T, element::type, Convertible>()) {
						return index_element<I - 1, element::type>();
					} else {
						return _find_first<T, Convertible, I - 1>();
					}
				} else {
					return bad_element();
				}
			}

			template<size_t CurIdx, typename Arr, size_t DstIdx, typename T, typename CurType, typename... OtherTypes>
			static constexpr auto _insert() {
				using arr = std::conditional_t<CurIdx == DstIdx, Arr::template push_back<T, CurType>, Arr::template push_back<CurType>>;

				if constexpr (CurIdx + 1 == sizeof...(Values)) {
					return arr();
				} else {
					return _erase<CurIdx + 1, arr, DstIdx, OtherTypes...>();
				}
			}

			template<size_t I, typename T>
			static constexpr auto _insert() {
				if constexpr (I < sizeof...(Values)) {
					return _insert<0, array<>, I, T, Values...>();
				} else {
					return array<Values..., T>();
				}
			}

			template<size_t I, typename CurArr, typename CancatArr>
			static constexpr auto _cancat() {
				if constexpr (I < CancatArr::size) {
					using arr = CurArr::template push_back<CancatArr::template at<I>::type>;
					return _cancat<I + 1, arr, CancatArr>();
				} else {
					return CurArr();
				}
			}

			template<typename CancatArr>
			static constexpr auto _cancat() {
				if constexpr (sizeof...(Values) > 0) {
					if constexpr (CancatArr::size > 0) {
						return _cancat<0, array<Values...>, CancatArr>();
					} else {
						return array<Values...>();
					}
				} else {
					return CancatArr();
				}
			}

			template<size_t CurIdx, typename Arr, size_t TargetIdx, typename CurType, typename... OtherTypes>
			static constexpr auto _erase() {
				if constexpr (CurIdx == TargetIdx) {
					if constexpr (CurIdx + 1 == sizeof...(Values)) {
						return Arr();
					} else {
						return _erase<CurIdx + 1, Arr, TargetIdx, OtherTypes...>();
					}
				} else if constexpr (CurIdx < sizeof...(Values)) {
					using arr = Arr::template push_back<CurType>;

					if constexpr (CurIdx + 1 == sizeof...(Values)) {
						return arr();
					} else {
						return _erase<CurIdx + 1, arr, TargetIdx, OtherTypes...>();
					}
				} else {
					return Arr();
				}
			}

			template<size_t TargetIdx>
			static constexpr auto _erase() {
				if constexpr (TargetIdx < sizeof...(Values)) {
					return _erase<0, array<>, TargetIdx, Values...>();
				} else {
					return array<Values...>();
				}
			}

			///*
			static constexpr auto _pop_back() {
				if constexpr (sizeof...(Values) > 0) {
					return _erase<sizeof...(Values) - 1>();
				} else {
					return void();
				}
			}
			//*/

		public:
			static constexpr size_t size = sizeof...(Values);

			//index_element<Index, Type> or bad_element
			template<size_t I>
			using at = decltype(_at<I>());

			//new array<...>
			template<size_t I, typename T>
			using set = decltype(_set<I, T>());

			//index_element<Index, Type> or bad_element
			template<typename T, bool Convertible = false>
			using find_first = decltype(_find_first<T, Convertible, 0>());

			//index_element<Index, Type> or bad_element
			template<typename T, bool Convertible = false>
			using find_last = decltype(_find_last<T, Convertible, sizeof...(Values)>());

			template<typename T, bool Convertible = false>
			using has = std::bool_constant<!std::same_as<find_first<T, Convertible>, bad_element>>;

			//new array<...>
			template<typename... Args>
			using push_back = array<Values..., Args...>;

			//new array<...>
			template<size_t I, typename T>
			using insert = decltype(_insert<I, T>);

			template<derived_from_template<statical::array> Arr>
			using concat = decltype(_cancat<Arr>());

			//new array<...>
			template<size_t I>
			using erase = decltype(_erase<I>());

			//new array<...>
			//using pop_back = typename array<Values...>::erase<sizeof...(Values) - 1>;
			using pop_back = std::conditional_t<sizeof...(Values) == 0, array<>, decltype(_pop_back())>;
			//using pop_back = decltype(_pop_back());
		};

		template<typename... Values>
		requires std::conjunction_v<is_base_of_template<pair_element, Values>...>
			struct map {
			private:
				template<typename... Args>
				using _push_back = map<Values..., Args...>;

				template<typename K, bool Convertible>
				static constexpr auto _find() {
					using arr = array<Values::first...>;
					using element = arr::template find_first<K, Convertible>;

					if constexpr (std::same_as<element, bad_element>) {
						return bad_element();
					} else {
						return array<Values...>::at<element::index>::type();
					}
				}

				template<typename Pair, bool Convertible>
				static constexpr auto _insert() {
					if constexpr (sizeof...(Values) > 0) {
						using arr = array<Values::first...>;
						using element = arr::template find_first<Pair::first, Convertible>;

						if constexpr (std::same_as<element, bad_element>) {
							return map<Values..., Pair>();
						} else {
							return _insert<0, map<>, element::index, Pair, Values...>();
						}
					} else {
						return map<Pair>();
					}
				}

				template<size_t CurIdx, typename M, size_t DstIdx, typename Pair, typename CurPair, typename... OtherPairs>
				static constexpr auto _insert() {
					using m = M::template _push_back<std::conditional_t<CurIdx == DstIdx, Pair, CurPair>>;

					if constexpr (CurIdx + 1 == sizeof...(Values)) {
						return m();
					} else {
						return _insert<CurIdx + 1, m, DstIdx, Pair, OtherPairs...>();
					}
				}

				template<size_t CurIdx, typename M, size_t TargetIdx, typename CurType, typename... OtherTypes>
				static constexpr auto _erase() {
					if constexpr (CurIdx == TargetIdx) {
						if constexpr (CurIdx + 1 == sizeof...(Values)) {
							return M();
						} else {
							return _erase<CurIdx + 1, M, TargetIdx, OtherTypes...>();
						}
					} else if constexpr (CurIdx < sizeof...(Values)) {
						using m = M::template _push_back<CurType>;

						if constexpr (CurIdx + 1 == sizeof...(Values)) {
							return m();
						} else {
							return _erase<CurIdx + 1, m, TargetIdx, OtherTypes...>();
						}
					} else {
						return M();
					}
				}

				template<typename K, bool Convertible>
				static constexpr auto _erase() {
					if constexpr (sizeof...(Values) > 0) {
						using arr = array<Values::first...>;
						using element = arr::template find_first<K, Convertible>;

						if constexpr (std::same_as<element, bad_element>) {
							return map<Values...>();
						} else {
							return _erase<0, map<>, element::index, Values...>();
						}
					} else {
						return map<Values...>();
					}
				}

			public:
				static constexpr size_t size = sizeof...(Values);

				//pair_element<K, V> or bad_element
				template<typename K, bool Convertible = false>
				using find = decltype(_find<K, Convertible>());

				template<typename K, bool Convertible = false>
				using has = std::bool_constant<!std::same_as<find<K, Convertible>, bad_element>>;

				//new map<...>
				template<typename K, typename V, bool Convertible = false>
				using insert = decltype(_insert<pair_element<K, V>, Convertible>());

				//new map<...>
				template<typename K, bool Convertible = false>
				using erase = decltype(_erase<K, Convertible>());
		};

	}


	template<typename T>
	concept Integral = std::is_integral<T>::value;

	template<Integral T>
	T Add(T a, T b) {
		return b;
	}

	template<typename R>
	requires requires { typename StringDataType<std::remove_cvref_t<R>>; }
	//requires StringData<std::remove_cvref_t<R>>
	inline auto& SRK_CALL operator+=(std::u8string& left, R&& right) {
		if constexpr (std::same_as<std::remove_cvref_t<R>, std::string>) {
			left += (const std::u8string&)right;
		} else if constexpr (std::same_as<std::remove_cvref_t<R>, std::string_view>) {
			left += (const std::u8string_view&)right;
		} else if constexpr (std::convertible_to<std::remove_cvref_t<R>, char const*>) {
			left += (const char8_t*)right;
		}

		return left;
	}

	template <std::unsigned_integral T>
	constexpr int32_t Popcount_fallback(T val) noexcept {
		constexpr auto digits = std::numeric_limits<T>::digits;
		val = (T)(val - ((val >> 1) & (T)(0x5555'5555'5555'5555ull)));
		val = (T)((val & (T)(0x3333'3333'3333'3333ull)) + ((val >> 2) & (T)(0x3333'3333'3333'3333ull)));
		val = (T)((val + (val >> 4)) & (T)(0x0F0F'0F0F'0F0F'0F0Full));
		for (int32_t shiftDigits = 8; shiftDigits < digits; shiftDigits <<= 1) val += (T)(val >> shiftDigits);
		return val & (T)(digits + digits - 1);
	}

	template<typename T>
	requires ConvertibleString8Data<std::remove_cvref_t<T>>
		inline std::conditional_t<String8View<std::remove_cvref_t<T>>, std::remove_reference_t<T>&, ConvertToString8ViewType<std::remove_cvref_t<T>>> to_string8_view(T&& val) {
		if constexpr (String8View<std::remove_cvref_t<T>>) {
			return val;
		} else {
			return std::move(ConvertToString8ViewType<std::remove_cvref_t<T>>(std::forward<T>(val)));
		}
	}


	inline constexpr uint8_t SRK_CALL byteswap1(uint8_t val) {
		return val;
	}

	inline constexpr uint16_t SRK_CALL byteswap1(uint16_t val) {
		if (std::is_constant_evaluated()) {
			return uint16_t(val << 8) | uint16_t(val >> 8);
		} else {
#if SRK_COMPILER == SRK_COMPILER_MSVC
			return _byteswap_ushort(val);
#elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
			return __builtin_bswap16(val);
#else
			return uint16_t(val << 8) | uint16_t(val >> 8);
#endif
		}
	}

	inline constexpr uint32_t SRK_CALL byteswap1(uint32_t val) {
		if (std::is_constant_evaluated()) {
			return (val & 0x000000FFU << 24) | (val & 0x0000FF00U << 8) | (val & 0x00FF0000U >> 8) | (val & 0xFF000000U >> 24);
		} else {
#if SRK_COMPILER == SRK_COMPILER_MSVC
			return _byteswap_ulong(val);
#elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
			return __builtin_bswap32(val);
#else
			return (val & 0x000000FFU << 24) | (val & 0x0000FF00U << 8) | (val & 0x00FF0000U >> 8) | (val & 0xFF000000U >> 24);
#endif
		}
	}

	inline constexpr uint64_t SRK_CALL byteswap1(uint64_t val) {
		if (std::is_constant_evaluated()) {
			uint64_t Hi = byteswap1(uint32_t(val));
			return (Hi << 32) | byteswap1(uint32_t(val >> 32));
		} else {
#if SRK_COMPILER == SRK_COMPILER_MSVC
			return _byteswap_uint64(val);
#elif SRK_COMPILER == SRK_COMPILER_GCC || SRK_COMPILER == SRK_COMPILER_CLANG
			return __builtin_bswap64(val);
#else
			uint64_t Hi = byteswap1(uint32_t(val));
			return (Hi << 32) | byteswap1(uint32_t(val >> 32));
#endif
		}
	}


	template<typename T, template<typename> typename... Conditions>
	concept logic_or = std::disjunction_v<Conditions<T>...>;

	template<typename T, template<typename> typename... Conditions>
	struct is_logic_or : std::bool_constant<logic_or<T, Conditions...>> {};


	template<template<auto...> typename Base, typename Derived>
	struct is_base_of_nontype_template {
	private:
		struct _impl {
			template<auto... Args> static constexpr std::true_type test(const Base<Args...>*);
			static constexpr std::false_type test(...);
			using type = decltype(test(std::declval<Derived*>()));
		};

	public:
		static constexpr bool value = _impl::type::value;
	};
	template<typename Derived, template<auto...> typename Base> concept derived_from_nontype_template = is_base_of_nontype_template<Base, Derived>::value;

	template<size_t I>
	struct placeholder {
		static constexpr size_t position = I;
	};

	template<typename... Parameters>
	struct parameter_pack {
	private:
		template<typename T>
		using push = parameter_pack<Parameters..., T>;

		template<typename PP, typename Tuple, typename Cur>
		struct _replace_placeholder {
			using type = typename PP::template push<Cur>;
		};

		template<typename PP, typename Tuple, derived_from_nontype_template<placeholder> Cur>
		struct _replace_placeholder<PP, Tuple, Cur> {
			using type = typename PP::template push<TupleTryAtType<Cur::position, Cur, Tuple>>;
		};

		template<typename PP, typename Tuple, typename... Args>
		struct _try_replace_placeholders;

		template<typename PP, typename Tuple>
		struct _try_replace_placeholders<PP, Tuple> {
			using type = PP;
		};

		template<typename PP, typename Tuple, typename Cur, typename... Others>
		struct _try_replace_placeholders<PP, Tuple, Cur, Others...> : _try_replace_placeholders<typename _replace_placeholder<PP, Tuple, Cur>::type, Tuple, Others...> {};

	public:
		template<template<typename...> typename Tmpl>
		using substitute_template = Tmpl<Parameters...>;

		template<typename... Values>
		using replace_placeholders = typename _try_replace_placeholders<parameter_pack<>, std::tuple<Values...>, Parameters...>::type;
	};


	template<typename T>
	struct is_parameter_pack {
	private:
		struct _impl {
			template<typename... Args> static constexpr std::true_type test(const typename parameter_pack<Args...>*);
			static constexpr std::false_type test(...);
			using type = decltype(test(std::declval<T*>()));
		};

	public:
		static constexpr bool value = _impl::type::value;
	};
	template<typename T> concept parameter_pack_c = is_parameter_pack<T>::value;


	template<template<typename...> typename Tmpl, parameter_pack_c ParameterPack>
	struct packaged_template {
		template<typename... Parameters>
		using type = typename ParameterPack::template replace_placeholders<Parameters...>::template substitute_template<Tmpl>::type;
		//using type = ParameterPack::template substitute_template<Tmpl>;

		template<typename... Parameters>
		static constexpr auto value = ParameterPack::template replace_placeholders<Parameters...>::template substitute_template<Tmpl>::value;
	};


	template<typename T>
	struct is_packaged_template {
	private:
		struct _impl {
			template<template<typename...> typename Tmpl, parameter_pack_c ParameterPack> static constexpr std::true_type test(const typename packaged_template<Tmpl, ParameterPack>*);
			static constexpr std::false_type test(...);
			using type = decltype(test(std::declval<T*>()));
		};

	public:
		static constexpr bool value = _impl::type::value;
	};
	template<typename T> concept packaged_template_c = is_packaged_template<T>::value;


	//template<typename T, typename Condition, typename... TypeModifiers, typename = std::enable_if_t<is_packaged_template<Condition>::value>> concept packaged_concept1 = Condition<typename multi_modification<T, Modifiers...>::type>::value<T>;
}