#pragma once

#include "srk/Core.h"
#include "srk/TypeTraits.h"

namespace srk {
	template<typename T, typename... Types> concept SameAnyOf = IsSameAnyOf<T, Types...>::value;
	template<typename T, typename Tuple> concept SameAnyOfInTuple = TUPLE_FIND_FIRST_VALUE<T, Tuple> != TupleFindFirst<T, Tuple>::BAD_INDEX;
	template<typename T, typename... Types> concept SameAllOf = IsSameAllOf<T, Types...>::value;
;

	template<auto Target, auto... Values> concept EqualAnyOf = IsEqualAnyOf<Target, Values...>::value;


	template<typename T, typename... Types> concept ConvertibleAnyOf = IsConvertibleAnyOf<T, Types...>::value;
	template<typename T, typename... Types> concept ConvertibleAllOf = IsConvertibleAllOf<T, Types...>::value;


	template<typename T> concept Iterable = requires(T & t) { t.begin(); t.end(); };


	template<typename T> concept ScopedEnum = std::is_scoped_enum_v<T>;
	template<typename T> concept Boolean = std::same_as<T, bool>;
	template<typename T> concept NullPointer = std::is_null_pointer_v<T>;
	template<typename T> concept MemberFunctionPointer = std::is_member_function_pointer_v<T>;
	template<typename Derived, typename Base> concept NullPointerOrDerivedFrom = NullPointer<Derived> || std::derived_from<Derived, Base>;
	template<typename T, typename R, typename... Args> concept InvocableResult = std::is_invocable_r_v<R, T, Args...>;
	template<typename T, typename ResultTuple, typename... Args> concept InvocableAnyOfResult = std::invocable<T, Args...> && SameAnyOfInTuple<std::invoke_result_t<T, Args...>, ResultTuple>;


	template<typename T> concept Arithmetic = std::is_arithmetic_v<T>;
}