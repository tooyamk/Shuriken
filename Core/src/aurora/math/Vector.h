#pragma once

#include "aurora/Global.h"
#include "aurora/math/Math.h"

namespace aurora {
	template<size_t N, typename T>
	class Vector {
	public:
		static constexpr size_t DIMENSION = N;
		using ElementType = T;
		
		using Data = T[N];

		Vector() {
			memset(this, 0, sizeof(T) * N);
		}

		Vector(const no_init&) {}

		template<size_t L, typename K>
		requires std::convertible_to<K, T>
		Vector(const Vector<L, K>& vec) {
			set(vec.data);
		}

		template<size_t L, typename K>
		requires std::convertible_to<K, T>
		Vector(Vector<L, K>&& vec) noexcept {
			set(vec.data);
		}

		template<size_t L, typename K>
		requires std::convertible_to<K, T>
		Vector(const K(&values)[L]) {
			set(values);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		Vector(const K* values, size_t len) {
			if (N > len) {
				for (decltype(len) i = 0; i < len; ++i) data[i] = values[i];
				memset(data + len, 0, sizeof(T) * (N - len));
			} else {
				for (decltype(len) i = 0; i < N; ++i) data[i] = values[i];
			}
		}

		template<typename K>
		requires std::convertible_to<K, T>
		Vector(const std::initializer_list<const K>& list) : Vector(list.begin(), list.size()) {
		}

		template<typename... Args>
		requires convertible_all_to<T, Args...>
		Vector(Args&&... args) {
			set(std::forward<Args>(args)...);
		}

		inline AE_CALL operator Data&() {
			return data;
		}

		inline AE_CALL operator const Data&() const {
			return data;
		}

		template<std::integral I>
		inline T& AE_CALL operator[](I i) {
			return data[i];
		}

		template<std::integral I>
		inline const T& AE_CALL operator[](I i) const {
			return data[i];
		}

		template<size_t L, typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL operator=(const Vector<L, K>& value) {
			return set(value.data);
		}

		template<size_t L, typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL operator=(Vector<L, K>&& value) noexcept {
			return set(value.data);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline bool AE_CALL operator==(const K& value) const {
			return Math::isEqual(data, value);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline bool AE_CALL operator==(const Vector<N, K>& value) const {
			if constexpr (std::same_as<T, K>) {
				return !memcmp(data, value.data, sizeof(Data));
			} else {
				return Math::isEqual(data, value.data);
			}
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline bool AE_CALL operator!=(const K& value) const {
			return !Math::isEqual(data, value);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline bool AE_CALL operator!=(const Vector<N, K>& value) const {
			if constexpr (std::same_as<T, K>) {
				return memcmp(data, value.data, sizeof(Data));
			} else {
				return !Math::isEqual(data, value.data);
			}
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator+=(const K& value) {
			add(value);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator+=(const K(&values)[N]) {
			add(values);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator+=(const Vector<N, K>& vec) {
			add(vec);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator-=(const K& value) {
			sub(value);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator-=(const K(&values)[N]) {
			sub(values);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator-=(const Vector<N, K>& vec) {
			sub(vec);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator*=(const K& value) {
			mul(value);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator*=(const K(&values)[N]) {
			mul(values);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator*=(const Vector<N, K>& vec) {
			mul(vec);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator/=(const K& value) {
			div(value);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator/=(const K(&values)[N]) {
			div(values);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL operator/=(const Vector<N, K>& vec) {
			div(vec);
		}

		inline Vector<N, T> AE_CALL operator-() const {
			Vector<N, T> val(no_init_v);
			for (decltype(N) i = 0; i < N; ++i) val.data[i] = -data[i];
			return val;
		}

		inline constexpr size_t AE_CALL getSize() const {
			return N;
		}

		template<size_t L, typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL set(const Vector<L, K>& vec) {
			return set(vec.data);
		}

		template<size_t L, typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL set(const K(&values)[L]) {
			if constexpr (std::same_as<T, K>) {
				if constexpr (L >= N) {
					memcpy(data, values, sizeof(T) * N);
				} else {
					memcpy(data, values, sizeof(T) * L);
				}
			} else {
				if constexpr (L >= N) {
					for (decltype(N) i = 0; i < N; ++i) data[i] = values[i];
				} else {
					for (decltype(N) i = 0; i < L; ++i) data[i] = values[i];
				}
			}

			if constexpr (L < N) memset(data + L, 0, sizeof(T) * (N - L));

			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL set(const K* values, size_t len) {
			if (len >= N) {
				for (decltype(N) i = 0; i < N; ++i) data[i] = values[i];
			} else {
				for (decltype(len) i = 0; i < len; ++i) data[i] = values[i];
				memset(data + len, 0, sizeof(T) * (N - len));
			}

			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL set(const std::initializer_list<const K>& list) {
			return set(list.begin(), list.size());
		}

		template<typename... Args>
		requires convertible_all_to<T, Args...>
		inline Vector& AE_CALL set(Args&&... args) {
			if constexpr (N > 0) {
				if constexpr (sizeof...(args) == 1) {
					_setAll(std::forward<Args>(args)...);
				} else if constexpr (N >= sizeof...(args)) {
					size_t i = 0;
					((data[i++] = args), ...);
					memset(data + sizeof...(args), 0, sizeof(T) * (N - sizeof...(args)));
				} else {
					_set<0>(std::forward<Args>(args)...);
				}
			}
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline void AE_CALL copyTo(K(&dst)[N]) const {
			if constexpr (std::same_as<T, K>) {
				memcpy(dst, data, sizeof(T) * N);
			} else {
				for (decltype(N) i = 0; i < N; ++i) dst[i] = data[i];
			}
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL add(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] += value;
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL add(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] += values[i];
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL add(const Vector<N, K>& vec) {
			return add(vec.data);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL sub(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] -= value;
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL sub(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] -= values[i];
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL sub(const Vector<N, K>& vec) {
			return sub(vec.data);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL mul(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] *= value;
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL mul(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] *= values[i];
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL mul(const Vector<N, K>& vec) {
			return mul(vec.data);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL div(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] /= value;
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL div(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] /= values[i];
			return *this;
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline Vector& AE_CALL div(const Vector<N, K>& vec) {
			return div(vec.data);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline bool AE_CALL isEqual(const K& value) const {
			return Math::isEqual(data, value);
		}

		template<typename K, typename S>
		requires std::convertible_to<K, T>
		inline bool AE_CALL isEqual(const K& value, const S& tolerance) const {
			return Math::isEqual(data, value, tolerance);
		}

		template<typename K>
		requires std::convertible_to<K, T>
		inline bool AE_CALL isEqual(const Vector<N, K>& value) const {
			return Math::isEqual(data, value.data);
		}

		template<typename K, typename S>
		requires std::convertible_to<K, T>
		inline bool AE_CALL isEqual(const Vector<N, K>& value, const S& tolerance) const {
			return Math::isEqual(data, value.data, tolerance);
		}

		template<std::integral... Indices>
		inline const Vector<sizeof...(Indices), T> AE_CALL components(Indices&&... indices) const {
			Vector<sizeof...(Indices), T> v(no_init_v);
			decltype(N) i = 0;
			((v.data[i++] = data[indices]), ...);
			return std::move(v);
		}

		inline Vector& AE_CALL normalize() {
			Math::normalize(data);
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
				for (decltype(N) i = 1; i < N; ++i) {
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
				for (decltype(N) i = 1; i < N; ++i) {
					if (val < data[i]) val = data[i];
				}
				return val;
			} else {
				return T(0);
			}
		}

		template<size_t COUNT = N>
		inline Vector<COUNT, T>& AE_CALL cast() {
			return *(Vector<COUNT, T>*)data;
		}

		template<size_t COUNT = N>
		inline const Vector<COUNT, T>& AE_CALL cast() const {
			return *(const Vector<COUNT, T>*)data;
		}

		template<size_t COUNT = N>
		inline Vector<COUNT, T>& AE_CALL cast(size_t start) {
			return *(Vector<COUNT, T>*)(data + start);
		}

		template<size_t COUNT = N>
		inline const Vector<COUNT, T>& AE_CALL cast(size_t start) const {
			return *(const Vector<COUNT, T>*)(data + start);
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
		template<size_t I, typename... Args>
		inline void AE_CALL _set(const T& value, Args&&... args) {
			if constexpr (I < N) {
				data[I] = value;
				_set<I + 1>(std::forward<Args>(args)...);
			}
		}

		template<typename V>
		inline void AE_CALL _setAll(V&& v) {
			for (decltype(N) i = 0; i < N; ++i) data[i] = v;
		}
	};

	template<size_t N, typename T> const Vector<N, T> Vector<N, T>::ZERO = Vector<N, T>(Math::NUMBER_0<T>);
	template<size_t N, typename T> const Vector<N, T> Vector<N, T>::ONE = Vector<N, T>(Math::NUMBER_1<T>);

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> AE_CALL operator+(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
		v += v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> AE_CALL operator+(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
			v += v2;
			return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> AE_CALL operator+(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
		v += v2;
		return v;
	}

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> AE_CALL operator-(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> AE_CALL operator-(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> AE_CALL operator-(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> AE_CALL operator*(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> AE_CALL operator*(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> AE_CALL operator*(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> AE_CALL operator/(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> AE_CALL operator/(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
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