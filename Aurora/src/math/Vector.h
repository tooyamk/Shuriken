#pragma once

#include "base/LowLevel.h"
#include "math/Math.h"

namespace aurora {
	template<ui32 N, typename T>
	class AE_TEMPLATE_DLL Vector {
	public:
		using Vec = Vector<N, T>;
		using Data = T[N];

		Vec() {
			memset(this, 0, sizeof(T) * N);
		}

		Vec(const T value) {
			set(value);
		}

		Vec(const Vec& vec) {
			set(vec);
		}

		Vec(Vec&& vec) {
			set(vec);
		}

		Vec(const T(&values)[N]) {
			set(values);
		}

		Vec(const T* values, ui32 len) {
			set(values, len);
			if (N > len) memset(data + len, 0, sizeof(T) * (N - len));
		}

		Vec(const std::initializer_list<const T>& list) {
			set(list);
			if (N > list.size()) memset(data + list.size(), 0, sizeof(T) * (N - list.size()));
		}

		template<typename... Args, typename = typename std::enable_if_t<are_all_convertible_v<Args..., T>>>
		Vec(Args... args) {
			set(args...);
			if constexpr (N > sizeof...(args)) memset(data + sizeof...(args), 0, sizeof(T) * (N - sizeof...(args)));
		}

		inline AE_CALL operator Data&() {
			return data;
		}

		inline  T& AE_CALL operator[](i32 i) {
			return data[i];
		}

		inline const T& AE_CALL operator[](i32 i) const {
			return data[i];
		}

		inline bool AE_CALL operator==(const T value) const {
			return Math::isEqual<N, T>(data, value);
		}

		inline bool AE_CALL operator==(const Vec& value) const {
			return Math::isEqual<N, T>(data, value.data);
		}

		inline bool AE_CALL operator!=(const T value) const {
			return !Math::isEqual<N, T>(data, value);
		}

		inline bool AE_CALL operator!=(const Vec& value) const {
			return !Math::isEqual<N, T>(data, value.data);
		}

		inline void AE_CALL operator+=(const T value) {
			add(value);
		}

		inline void AE_CALL operator+=(const T(&values)[N]) {
			add(values);
		}

		inline void AE_CALL operator+=(const Vec& vec) {
			add(vec);
		}

		inline void AE_CALL operator-=(const T value) {
			sub(value);
		}

		inline void AE_CALL operator-=(const T(&values)[N]) {
			sub(values);
		}

		inline void AE_CALL operator-=(const Vec& vec) {
			sub(vec);
		}

		inline void AE_CALL operator*=(const T value) {
			mul(value);
		}

		inline void AE_CALL operator*=(const T(&values)[N]) {
			mul(values);
		}

		inline void AE_CALL operator*=(const Vec& vec) {
			mul(vec);
		}

		inline void AE_CALL operator/=(const T value) {
			div(value);
		}

		inline void AE_CALL operator/=(const T(&values)[N]) {
			div(values);
		}

		inline void AE_CALL operator/=(const Vec& vec) {
			div(vec);
		}

		inline Vec& AE_CALL set(const T value) {
			for (ui32 i = 0; i < N; ++i) data[i] = value;
			return *this;
		}

		inline Vec& AE_CALL set(const Vec& vec) {
			return set(vec.data);
		}

		inline Vec& AE_CALL set(const T(&values)[N]) {
			for (ui32 i = 0; i < N; ++i) data[i] = values[i];
			return *this;
		}

		inline Vec& AE_CALL set(const T* values, ui32 len) {
			for (ui32 i = 0, n = N > len ? len : N; i < n; ++i) data[i] = values[i];
			return *this;
		}

		inline Vec& AE_CALL set(const std::initializer_list<const T>& list) {
			return set(list.begin(), list.size());
		}

		template<typename... Args, typename = typename std::enable_if_t<are_all_convertible_v<Args..., T>>>
		inline Vec& AE_CALL set(Args... args) {
			if constexpr (N > 0) {
				if constexpr (N >= sizeof...(args)) {
					ui32 i = 0;
					((data[i++] = args), ...);
				} else {
					_set(0, args...);
				}
			}
			return *this;
		}

		inline Vec& AE_CALL add(const T value) {
			for (ui32 i = 0; i < N; ++i) data[i] += value;
			return *this;
		}

		inline Vec& AE_CALL add(const T(&values)[N]) {
			for (ui32 i = 0; i < N; ++i) data[i] += values[i];
			return *this;
		}

		inline Vec& AE_CALL add(const Vec& vec) {
			return add(vec.data);
		}

		inline Vec& AE_CALL sub(const T value) {
			for (ui32 i = 0; i < N; ++i) data[i] -= value;
			return *this;
		}

		inline Vec& AE_CALL sub(const T(&values)[N]) {
			for (ui32 i = 0; i < N; ++i) data[i] -= values[i];
			return *this;
		}

		inline Vec& AE_CALL sub(const Vec& vec) {
			return sub(vec.data);
		}

		inline Vec& AE_CALL mul(const T value) {
			for (ui32 i = 0; i < N; ++i) data[i] *= value;
			return *this;
		}

		inline Vec& AE_CALL mul(const T(&values)[N]) {
			for (ui32 i = 0; i < N; ++i) data[i] *= values[i];
			return *this;
		}

		inline Vec& AE_CALL mul(const Vec& vec) {
			return mul(vec.data);
		}

		inline Vec& AE_CALL div(const T value) {
			for (ui32 i = 0; i < N; ++i) data[i] /= value;
			return *this;
		}

		inline Vec& AE_CALL div(const T(&values)[N]) {
			for (ui32 i = 0; i < N; ++i) data[i] /= values[i];
			return *this;
		}

		inline Vec& AE_CALL div(const Vec& vec) {
			return div(vec.data);
		}

		inline bool AE_CALL isEqual(const T value) const {
			return Math::isEqual<N, T>(data, value);
		}

		inline bool AE_CALL isEqual(const T value, const T tolerance) const {
			return Math::isEqual<N, T>(data, value, tolerance);
		}

		inline bool AE_CALL isEqual(const Vec& value) const {
			return Math::isEqual<N, T>(data, value.data);
		}

		inline bool AE_CALL isEqual(const Vec& value, const T tolerance) const {
			return Math::isEqual<N, T>(data, value.data, tolerance);
		}

		inline Vec& AE_CALL normalize() {
			Math::normalize<N, T, T>(data, data);
			return *this;
		}

		template<typename Ret = T>
		inline Ret AE_CALL getLengthSq() const {
			return Math::dot<Ret>(data, data);
		}

		template<typename Ret = T>
		inline Ret AE_CALL getLength() const {
			return std::sqrt(getLengthSq<Ret>());
		}

		inline T AE_CALL getMax() const {
			T max = data[0];
			for (ui32 i = 1; i < N; ++i) {
				if (max < data[i]) max = data[i];
			}
			return max;
		}

		template<typename Ret = T>
		inline Ret AE_CALL getMultiplies() const {
			return Math::multiplies<N, T, Ret>(data);
		}

		template<typename Ret = T>
		inline static void AE_CALL normalize(const Vec& v, Vector<N, Ret>& dst) {
			Math::normalize<N, T, Ret>(v.data, dst.data);
		}

		template<typename Ret = T>
		inline static Ret AE_CALL dot(const Vec& v1, const Vec& v2) {
			return Math::dot<Ret>(v1.data, v2.data);
		}

		template<typename Ret = T>
		inline static void AE_CALL lerp(const Vec& from, const Vec& to, f32 t, Vector<N, Ret>& dst) {
			Math::lerp<N>(from.data, to.data, t, dst.data);
		}

		static const Vec ZERO;
		static const Vec ONE;

		Data data;

	private:
		inline void AE_CALL _set(ui32 i, T value) {}

		template<typename... Args>
		inline void AE_CALL _set(ui32 i, T value, Args... args) {
			data[i] = value;
			if (i++ < N) _set(i, args...);
		}
	};

	template<ui32 N, typename T> using Vec = Vector<N, T>;

	template<ui32 N, typename T> const Vec<N, T> Vector<N, T>::ZERO = Vector<N, T>(Math::NUMBER_0<T>);
	template<ui32 N, typename T> const Vec<N, T> Vector<N, T>::ONE = Vector<N, T>(Math::NUMBER_1<T>);

	template<typename T> using Vec1 = Vec<1, T>;
	using Vec1f32 = Vec1<f32>;
	using Vec1i32 = Vec1<i32>;
	using Vec1ui32 = Vec1<ui32>;
	template<typename T> using Vec2 = Vec<2, T>;
	using Vec2f32 = Vec2<f32>;
	using Vec2i32 = Vec2<i32>;
	using Vec2ui32 = Vec2<ui32>;
	template<typename T> using Vec3 = Vec<3, T>;
	using Vec3f32 = Vec3<f32>;
	using Vec3i32 = Vec3<i32>;
	using Vec3ui32 = Vec3<ui32>;
	template<typename T> using Vec4 = Vec<4, T>;
	using Vec4f32 = Vec4<f32>;
	using Vec4i32 = Vec4<i32>;
	using Vec4ui32 = Vec4<ui32>;
}