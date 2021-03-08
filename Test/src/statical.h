#include "aurora/Global.h"

struct bad_type {};

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
		inline static constexpr std::optional<size_t> index = std::is_same_v<T, PrevType> ? std::make_optional<size_t>(I - 1) : std::nullopt;
	};

	template<size_t I, typename T, typename PrevType, typename CurType, typename... OtherTypes>
	struct _find<true, I, T, PrevType, CurType, OtherTypes...> : _find<!std::is_same_v<T, CurType> && (I + 1 < sizeof...(Types)), I + 1, T, CurType, OtherTypes...> {};

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
	template<template<typename...> typename Base, typename Derived>
	struct is_base_of_template {
	private:
		template<template<typename...> typename Base, typename Derived>
		struct _impl {
			template<typename... Args> static constexpr std::true_type test(const Base<Args...>*);
			static constexpr std::false_type test(...);
			using type = decltype(test(std::declval<Derived*>()));
		};

	public:
		static constexpr bool value = _impl<Base, Derived>::type::value;
	};

	template<template<typename...> typename Base, typename Derived>
	inline static constexpr bool is_base_of_template_v = is_base_of_template<Base, Derived>::value;

	struct bad_element {};

	template<auto Val, typename T = decltype(Val), typename = std::enable_if_t<std::is_convertible_v<decltype(Val), T>>>
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
	inline static constexpr bool is_base_of_nontype_value_v = is_base_of_nontype_value<T>::value;

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

		template<template<typename...> typename Arr>
		using concat = int;// decltype(concat<Arr>());

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
			if constexpr (is_base_of_nontype_value_v<T1>) {
				if constexpr (is_base_of_nontype_value_v<T2>) {
					if constexpr (Convertible) {
						if constexpr (!(std::is_convertible_v<T1::type, T2::type> || std::is_convertible_v<T2::type, T1::type>)) {
							return false;
						}
					} else {
						if constexpr (!std::is_same_v<T1, T2>) {
							return false;
						}
					}

					return T1::value == T2::value;
				} else {
					return false;
				}
			} else {
				if constexpr (is_base_of_nontype_value_v<T2>) {
					return false;
				} else {
					return Convertible ? (std::is_convertible_v<T1, T2> || std::is_convertible_v<T2, T1>) : std::is_same_v<T1, T2>;
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

		/*
		template<template<typename... Args> typename Arr>
		static constexpr auto _cancat() {
			return array<>();
			if constexpr (sizeof...(Values) > 0) {
				return _erase<sizeof...(Values) - 1>();
			} else {
				return Arr();
			}
		}
		*/

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

		/*
		static constexpr auto _pop_back() {
			if constexpr (sizeof...(Values) > 0) {
				return _erase<sizeof...(Values) - 1>();
			} else {
				return array<>();
			}
		}
		*/

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
		using has = std::bool_constant<!std::is_same_v<find_first<T, Convertible>, bad_element>>;

		//new array<...>
		template<typename... Args>
		using push_back = array<Values..., Args...>;

		//new array<...>
		template<size_t I, typename T>
		using insert = decltype(_insert<I, T>);

		//template<typename Arr, typename = std::enable_if_t<is_base_of_template_v<array, Arr>>>
		template<template<typename...> typename Arr>
		using concat = int;// decltype(concat<Arr>());

		//new array<...>
		template<size_t I>
		using erase = decltype(_erase<I>());

		//new array<...>
		//using pop_back = typename array<Values...>::erase<sizeof...(Values) - 1>;
	};

	template<typename... Values>
	requires requires { typename std::enable_if_t<std::conjunction_v<is_base_of_template<pair_element, Values>...>>; }
	struct map {
	private:
		template<typename... Args>
		using _push_back = map<Values..., Args...>;

		template<typename K, bool Convertible>
		static constexpr auto _find() {
			using arr = array<Values::first...>;
			using element = arr::template find_first<K, Convertible>;

			if constexpr (std::is_same_v<element, bad_element>) {
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

				if constexpr (std::is_same_v<element, bad_element>) {
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

				if constexpr (std::is_same_v<element, bad_element>) {
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
		using has = std::bool_constant<!std::is_same_v<find<K, Convertible>, bad_element>>;

		//new map<...>
		template<typename K, typename V, bool Convertible = false>
		using insert = decltype(_insert<pair_element<K, V>, Convertible>());

		//new map<...>
		template<typename K, bool Convertible = false>
		using erase = decltype(_erase<K, Convertible>());
	};
}