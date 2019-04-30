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
		}

		Vec(const std::initializer_list<const T>& list) {
			set(list);
		}

		inline operator Data&() {
			return data;
		}

		inline  T& operator[](i32 i) {
			return data[i];
		}

		inline const T& operator[](i32 i) const {
			return data[i];
		}

		inline bool operator==(const T value) const {
			return Math::isEqual<N, T>(data, value);
		}

		inline bool operator==(const Vec& value) const {
			return Math::isEqual<N, T>(data, value.data);
		}

		inline bool operator!=(const T value) const {
			return !Math::isEqual<N, T>(data, value);
		}

		inline bool operator!=(const Vec& value) const {
			return !Math::isEqual<N, T>(data, value.data);
		}

		inline Vec& AE_CALL set(const T value) {
			for (ui32 i = 0; i < N; ++i) data[i] = value;
			return *this;
		}

		inline Vec& AE_CALL set(const Vec& vec) {
			return set(vec.data, N);
		}

		inline Vec& AE_CALL set(const T(&values)[N]) {
			for (ui32 i = 0; i < N; ++i) data[i] = values[i];
			return *this;
		}

		inline Vec& AE_CALL set(const T* values, ui32 len) {
			auto n = N > len ? len : N;
			for (ui32 i = 0; i < n; ++i) data[i] = values[i];
			return *this;
		}

		inline Vec& AE_CALL set(const std::initializer_list<const T>& list) {
			return set(list.begin(), list.size());
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
		inline Ret AE_CALL getCumprod() const {
			Ret c = data[0];
			for (ui32 i = 1; i < N; ++i) c *= data[i];
			return c;
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
	};

	template<ui32 N, typename T> using Vec = Vector<N, T>;

	template<ui32 N, typename T> const Vec<N, T> Vector<N, T>::ZERO = Vector<N, T>(Math::NUMBER_O<T>);
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