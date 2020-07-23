#pragma once

#include "aurora/Global.h"
#include "aurora/math/Math.h"

namespace aurora {
	template<uint32_t N, typename T>
	class AE_CORE_TMPL_DLL Vector {
	public:
		using Data = T[N];

		template<uint32_t COUNT>
		using SliceType = T[COUNT];

		template<typename K>
		using ConvertibleType = typename std::enable_if_t<std::is_convertible_v<K, T>, K>;

		Vector() {
			memset(this, 0, sizeof(T) * N);
		}

		Vector(const NoInit&) {}

		template<typename K, typename = ConvertibleType<K>>
		Vector(const Vector<N, K>& vec) {
			set(vec.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		Vector(Vector<N, K>&& vec) noexcept {
			set(vec.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		Vector(const K(&values)[N]) {
			set(values);
		}

		template<typename K, typename = ConvertibleType<K>>
		Vector(const K* values, uint32_t len) {
			if (N > len) {
				for (uint32_t i = 0; i < len; ++i) data[i] = values[i];
				memset(data + len, 0, sizeof(T) * (N - len));
			} else {
				for (uint32_t i = 0; i < N; ++i) data[i] = values[i];
			}
		}

		template<typename K, typename = ConvertibleType<K>>
		Vector(const std::initializer_list<const K>& list) : Vector(list.begin(), list.size()) {
		}

		template<typename... Args, typename = typename std::enable_if_t<std::conjunction_v<std::is_convertible<Args, T>...>>>
		Vector(Args&&... args) {
			set(std::forward<Args>(args)...);
			if constexpr (N > sizeof...(args) && sizeof...(args) > 1) memset(data + sizeof...(args), 0, sizeof(T) * (N - sizeof...(args)));
		}

		inline AE_CALL operator Data&() {
			return data;
		}

		inline AE_CALL operator const Data&() const {
			return data;
		}

		inline T& AE_CALL operator[](int32_t i) {
			return data[i];
		}

		inline const T& AE_CALL operator[](int32_t i) const {
			return data[i];
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL operator=(const Vector<N, K>& value) {
			return set(value.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL operator=(Vector<N, K>&& value) noexcept {
			return set(value.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL operator==(const K& value) const {
			return Math::isEqual(data, value);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL operator==(const Vector<N, K>& value) const {
			return Math::isEqual(data, value.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL operator!=(const K& value) const {
			return !Math::isEqual(data, value);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL operator!=(const Vector<N, K>& value) const {
			return !Math::isEqual(data, value.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator+=(const K& value) {
			add(value);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator+=(const K(&values)[N]) {
			add(values);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator+=(const Vector<N, K>& vec) {
			add(vec);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator-=(const K& value) {
			sub(value);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator-=(const K(&values)[N]) {
			sub(values);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator-=(const Vector<N, K>& vec) {
			sub(vec);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator*=(const K& value) {
			mul(value);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator*=(const K(&values)[N]) {
			mul(values);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator*=(const Vector<N, K>& vec) {
			mul(vec);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator/=(const K& value) {
			div(value);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator/=(const K(&values)[N]) {
			div(values);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL operator/=(const Vector<N, K>& vec) {
			div(vec);
		}

		inline constexpr uint32_t AE_CALL getSize() const {
			return N;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL set(const Vector<N, K>& vec) {
			return set(vec.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL set(const K(&values)[N]) {
			if constexpr (std::is_same_v<T, K>) {
				memcpy(data, values, sizeof(T) * N);
			} else {
				for (uint32_t i = 0; i < N; ++i) data[i] = values[i];
			}

			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL set(const K* values, uint32_t len) {
			for (uint32_t i = 0, n = N > len ? len : N; i < n; ++i) data[i] = values[i];
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL set(const std::initializer_list<const K>& list) {
			return set(list.begin(), list.size());
		}

		template<typename... Args, typename = typename std::enable_if_t<std::conjunction_v<std::is_convertible<Args, T>...>>>
		inline Vector& AE_CALL set(Args&&... args) {
			if constexpr (N > 0) {
				if constexpr (sizeof...(args) == 1) {
					_setAll(std::forward<Args>(args)...);
				} if constexpr (N >= sizeof...(args)) {
					uint32_t i = 0;
					((data[i++] = args), ...);
				} else {
					_set<0>(std::forward<Args>(args)...);
				}
			}
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL copyTo(K(&dst)[N]) const {
			if constexpr (std::is_same_v<T, K>) {
				memcpy(dst, data, sizeof(T) * N);
			} else {
				for (uint32_t i = 0; i < N; ++i) dst[i] = data[i];
			}
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL add(const K& value) {
			for (uint32_t i = 0; i < N; ++i) data[i] += value;
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL add(const K(&values)[N]) {
			for (uint32_t i = 0; i < N; ++i) data[i] += values[i];
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL add(const Vector<N, K>& vec) {
			return add(vec.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL sub(const K& value) {
			for (uint32_t i = 0; i < N; ++i) data[i] -= value;
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL sub(const K(&values)[N]) {
			for (uint32_t i = 0; i < N; ++i) data[i] -= values[i];
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL sub(const Vector<N, K>& vec) {
			return sub(vec.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL mul(const K& value) {
			for (uint32_t i = 0; i < N; ++i) data[i] *= value;
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL mul(const K(&values)[N]) {
			for (uint32_t i = 0; i < N; ++i) data[i] *= values[i];
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL mul(const Vector<N, K>& vec) {
			return mul(vec.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL div(const K& value) {
			for (uint32_t i = 0; i < N; ++i) data[i] /= value;
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL div(const K(&values)[N]) {
			for (uint32_t i = 0; i < N; ++i) data[i] /= values[i];
			return *this;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline Vector& AE_CALL div(const Vector<N, K>& vec) {
			return div(vec.data);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL isEqual(const K& value) const {
			return Math::isEqual(data, value);
		}

		template<typename K, typename S, typename = ConvertibleType<K>>
		inline bool AE_CALL isEqual(const K& value, const S& tolerance) const {
			return Math::isEqual(data, value, tolerance);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL isEqual(const Vector<N, K>& value) const {
			return Math::isEqual(data, value.data);
		}

		template<typename K, typename S, typename = ConvertibleType<K>>
		inline bool AE_CALL isEqual(const Vector<N, K>& value, const S& tolerance) const {
			return Math::isEqual(data, value.data, tolerance);
		}

		inline Vector& AE_CALL normalize() {
			Math::normalize(data, data);
			return *this;
		}

		template<typename Ret = T>
		inline void AE_CALL normalize(Vector<N, Ret>& dst) const {
			Math::normalize<N, T, Ret>(data, dst.data);
		}

		template<typename Ret = T>
		inline Ret AE_CALL getLengthSq() const {
			return Math::dot<N, T, T, Ret>(data, data);
		}

		template<typename Ret = T>
		inline Ret AE_CALL getLength() const {
			return std::sqrt(getLengthSq<Ret>());
		}

		inline T AE_CALL getMin() const {
			if constexpr (N > 0) {
				T val = data[0];
				for (uint32_t i = 1; i < N; ++i) {
					if (val > data[i]) val = data[i];
				}
				return val;
			} else {
				return T(0);
			}
		}

		inline T AE_CALL getMax() const {
			if constexpr (N > 0) {
				T val = data[0];
				for (uint32_t i = 1; i < N; ++i) {
					if (val < data[i]) val = data[i];
				}
				return val;
			} else {
				return T(0);
			}
		}

		template<uint32_t COUNT = N>
		inline SliceType<COUNT>& AE_CALL slice() const {
			return (SliceType<COUNT>&)data;
		}

		template<uint32_t COUNT>
		inline SliceType<COUNT>& AE_CALL slice(uint32_t start) const {
			return (SliceType<COUNT>&)*(data + start);
		}

		template<typename Ret = T>
		inline Ret AE_CALL getMultiplies() const {
			return Math::multiplies<N, T, Ret>(data);
		}

		template<typename In, typename Ret = decltype((*(T*)0) + (*(In*)0))>
		inline Ret AE_CALL dot(const Vector<N, In>& v) const {
			return Math::dot<N, T, In, Ret>(data, v.data);
		}

		template<typename In1, typename In2, typename Ret>
		inline void AE_CALL lerp(const Vector<N, In1>& to, const In2& t, Vector<N, Ret>& dst) const {
			Math::lerp(data, to.data, t, dst.data);
		}

		static const Vector ZERO;
		static const Vector ONE;

		Data data;

	private:
		template<uint32_t I, typename... Args>
		inline void AE_CALL _set(const T& value, Args... args) {
			if constexpr (I < N) {
				data[I] = value;
				_set<I + 1>(std::forward<Args>(args)...);
			}
		}

		template<typename V>
		inline void AE_CALL _setAll(V&& v) {
			for (uint32_t i = 0; i < N; ++i) data[i] = v;
		}
	};

	template<uint32_t N, typename T> const Vector<N, T> Vector<N, T>::ZERO = Vector<N, T>(Math::NUMBER_0<T>);
	template<uint32_t N, typename T> const Vector<N, T> Vector<N, T>::ONE = Vector<N, T>(Math::NUMBER_1<T>);

	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> AE_CALL operator+(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
		v += v2;
		return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> AE_CALL operator+(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
			v += v2;
			return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> AE_CALL operator+(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
		v += v2;
		return v;
	}

	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> AE_CALL operator-(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> AE_CALL operator-(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> AE_CALL operator-(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}

	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> AE_CALL operator*(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> AE_CALL operator*(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> AE_CALL operator*(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}

	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> AE_CALL operator/(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> AE_CALL operator/(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}
	template<uint32_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> AE_CALL operator/(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}

	template<typename T> using Vec1 = Vector<1, T>;
	using Vec1f32 = Vec1<float32_t>;
	using Vec1i32 = Vec1<int32_t>;
	using Vec1ui32 = Vec1<uint32_t>;
	template<typename T> using Vec2 = Vector<2, T>;
	using Vec2f32 = Vec2<float32_t>;
	using Vec2i32 = Vec2<int32_t>;
	using Vec2ui32 = Vec2<uint32_t>;
	template<typename T> using Vec3 = Vector<3, T>;
	using Vec3f32 = Vec3<float32_t>;
	using Vec3i32 = Vec3<int32_t>;
	using Vec3ui32 = Vec3<uint32_t>;
	template<typename T> using Vec4 = Vector<4, T>;
	using Vec4f32 = Vec4<float32_t>;
	using Vec4i32 = Vec4<int32_t>;
	using Vec4ui32 = Vec4<uint32_t>;
}