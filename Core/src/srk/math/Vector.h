#pragma once

#include "srk/Global.h"
#include "srk/math/Math.h"

namespace srk {
	template<size_t N, typename T>
	class Vector {
	public:
		static constexpr size_t DIMENSION = N;
		using ElementType = T;
		
		using Data = T[N];

		template<std::convertible_to<T> K>
		struct All {
			All(const K& v) : value(v) {}
			T value;
		};

		Vector() {
			memset(this, 0, sizeof(T) * N);
		}

		Vector(nullptr_t) {}

		template<size_t L, std::convertible_to<T> K>
		Vector(const Vector<L, K>& vec) {
			set(vec.data);
		}

		template<size_t L, std::convertible_to<T> K>
		Vector(Vector<L, K>&& vec) noexcept {
			set(vec.data);
		}

		template<size_t L, std::convertible_to<T> K>
		Vector(const K(&values)[L]) {
			set(values);
		}

		template<std::convertible_to<T> K>
		Vector(const K* values, size_t len) {
			if (N > len) {
				for (decltype(len) i = 0; i < len; ++i) data[i] = values[i];
				memset(data + len, 0, sizeof(T) * (N - len));
			} else {
				for (decltype(len) i = 0; i < N; ++i) data[i] = values[i];
			}
		}

		template<std::convertible_to<T> K>
		Vector(const std::initializer_list<const K>& list) : Vector(list.begin(), list.size()) {
		}

		template<std::convertible_to<T>... Args>
		Vector(Args&&... args) {
			set(std::forward<Args>(args)...);
		}

		template<std::convertible_to<T> K>
		Vector(All<K>&& v) {
			setAll(v.value);
		}

		inline SRK_CALL operator Data&() {
			return data;
		}

		inline SRK_CALL operator const Data&() const {
			return data;
		}

		template<std::integral I>
		inline T& SRK_CALL operator[](I i) {
			return data[i];
		}

		template<std::integral I>
		inline const T& SRK_CALL operator[](I i) const {
			return data[i];
		}

		template<size_t L, std::convertible_to<T> K>
		inline Vector& SRK_CALL operator=(const Vector<L, K>& value) noexcept {
			return set(value.data);
		}

		template<size_t L, std::convertible_to<T> K>
		inline Vector& SRK_CALL operator=(Vector<L, K>&& value) noexcept {
			return set(value.data);
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL operator=(K&& v) noexcept {
			return setAll(std::forward(v));
		}

		template<std::convertible_to<T> K>
		inline bool SRK_CALL operator==(const K& value) const {
			return Math::equal(data, value);
		}

		template<std::convertible_to<T> K>
		inline bool SRK_CALL operator==(const Vector<N, K>& value) const {
			return Math::equal(data, value.data);
		}

		template<std::convertible_to<T> K>
		inline bool SRK_CALL operator!=(const K& value) const {
			return !Math::equal(data, value);
		}

		template<std::convertible_to<T> K>
		inline bool SRK_CALL operator!=(const Vector<N, K>& value) const {
			return !Math::equal(data, value.data);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator+=(const K& value) {
			add(value);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator+=(const K(&values)[N]) {
			add(values);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator+=(const Vector<N, K>& vec) {
			add(vec);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator-=(const K& value) {
			sub(value);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator-=(const K(&values)[N]) {
			sub(values);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator-=(const Vector<N, K>& vec) {
			sub(vec);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator*=(const K& value) {
			mul(value);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator*=(const K(&values)[N]) {
			mul(values);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator*=(const Vector<N, K>& vec) {
			mul(vec);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator/=(const K& value) {
			div(value);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator/=(const K(&values)[N]) {
			div(values);
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL operator/=(const Vector<N, K>& vec) {
			div(vec);
		}

		template<std::integral K>
		requires std::integral<T>
		inline void SRK_CALL operator<<=(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] <<= value;
		}

		template<std::integral K>
		requires std::integral<T>
		inline void SRK_CALL operator>>=(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] >>= value;
		}

		inline Vector<N, T> SRK_CALL operator-() const {
			Vector<N, T> val(nullptr);
			for (decltype(N) i = 0; i < N; ++i) val.data[i] = -data[i];
			return val;
		}

		inline constexpr size_t SRK_CALL getDimension() const {
			return N;
		}

		template<bool ResetOthers = true, size_t L, std::convertible_to<T> K >
		inline Vector& SRK_CALL set(const Vector<L, K>& vec) {
			return set<ResetOthers>(vec.data);
		}

		template<bool ResetOthers = true, size_t L, std::convertible_to<T> K>
		inline Vector& SRK_CALL set(const K(&values)[L]) {
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

			if constexpr (ResetOthers && L < N) memset(data + L, 0, sizeof(T) * (N - L));

			return *this;
		}

		template<bool ResetOthers = true, std::convertible_to<T> K>
		inline Vector& SRK_CALL set(const K* values, size_t len) {
			if (len >= N) {
				for (decltype(N) i = 0; i < N; ++i) data[i] = values[i];
			} else {
				for (decltype(len) i = 0; i < len; ++i) data[i] = values[i];
				if constexpr (ResetOthers) {
					memset(data + len, 0, sizeof(T) * (N - len));
				}
			}

			return *this;
		}

		template<bool ResetOthers = true, std::convertible_to<T> K>
		inline Vector& SRK_CALL set(const std::initializer_list<const K>& list) {
			return set<ResetOthers>(list.begin(), list.size());
		}

		template<bool ResetOthers = true, Math::DataDesc SrcDesc = nullptr, Math::DataDesc DstDesc = nullptr, std::convertible_to<T>... Args>
		inline Vector& SRK_CALL set(Args&&... args) {
			if constexpr (N > 0) {
				Math::copy<Math::DataDesc(SrcDesc, Math::DataType::VECTOR), Math::DataDesc(DstDesc, Math::DataType::VECTOR)>(data, std::forward<Args>(args)...);
				if constexpr (ResetOthers) {
					constexpr auto ddesc = DstDesc.manual(Math::Range(0, 0, N)).clamp(N);
					if constexpr (ddesc.range.realBeforeCount(N)) memset(data, 0, sizeof(T) * ddesc.range.realBeforeCount(N));

					constexpr auto afterCnt = ddesc.range.realAfterCount(N);
					if constexpr (afterCnt) memset(data + (N - afterCnt), 0, sizeof(T) * afterCnt);
				}
			}

			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL setAll(K&& v) {
			for (decltype(N) i = 0; i < N; ++i) data[i] = v;
			return *this;
		}

		template<std::convertible_to<T> K>
		inline void SRK_CALL copyTo(K(&dst)[N]) const {
			if constexpr (std::same_as<T, K>) {
				memcpy(dst, data, sizeof(T) * N);
			} else {
				for (decltype(N) i = 0; i < N; ++i) dst[i] = data[i];
			}
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL add(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] += value;
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL add(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] += values[i];
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL add(const Vector<N, K>& vec) {
			return add(vec.data);
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL sub(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] -= value;
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL sub(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] -= values[i];
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL sub(const Vector<N, K>& vec) {
			return sub(vec.data);
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL mul(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] *= value;
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL mul(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] *= values[i];
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL mul(const Vector<N, K>& vec) {
			return mul(vec.data);
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL div(const K& value) {
			for (decltype(N) i = 0; i < N; ++i) data[i] /= value;
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL div(const K(&values)[N]) {
			for (decltype(N) i = 0; i < N; ++i) data[i] /= values[i];
			return *this;
		}

		template<std::convertible_to<T> K>
		inline Vector& SRK_CALL div(const Vector<N, K>& vec) {
			return div(vec.data);
		}

		template<std::convertible_to<T> K>
		inline bool SRK_CALL equal(const K& value) const {
			return Math::equal(data, value);
		}

		template<std::convertible_to<T> K, typename S>
		inline bool SRK_CALL equal(const K& value, const S& tolerance) const {
			return Math::equal(data, value, tolerance);
		}

		template<std::convertible_to<T> K>
		inline bool SRK_CALL equal(const Vector<N, K>& value) const {
			return Math::equal(data, value.data);
		}

		template<std::convertible_to<T> K, typename S>
		inline bool SRK_CALL equal(const Vector<N, K>& value, const S& tolerance) const {
			return Math::equal(data, value.data, tolerance);
		}

		template<std::integral... Indices>
		inline const Vector<sizeof...(Indices), T> SRK_CALL components(Indices&&... indices) const {
			Vector<sizeof...(Indices), T> v(nullptr);
			decltype(N) i = 0;
			((v.data[i++] = data[indices]), ...);
			return std::move(v);
		}

		inline Vector& SRK_CALL normalize() {
			Math::normalize(data);
			return *this;
		}

		template<typename Ret = T>
		inline void SRK_CALL normalize(Vector<N, Ret>& dst) const {
			Math::normalize<N, T, Ret>(data, dst.data);
		}

		template<typename Ret = T>
		inline Ret SRK_CALL getLengthSq() const {
			return Math::dot<N, T, T, Ret>(data, data);
		}

		template<typename Ret = T>
		inline Ret SRK_CALL getLength() const {
			return std::sqrt(getLengthSq<Ret>());
		}

		inline T SRK_CALL getMin() const {
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

		inline T SRK_CALL getMax() const {
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
		inline Vector<COUNT, T>& SRK_CALL cast() {
			return *(Vector<COUNT, T>*)data;
		}

		template<size_t COUNT = N>
		inline const Vector<COUNT, T>& SRK_CALL cast() const {
			return *(const Vector<COUNT, T>*)data;
		}

		template<size_t COUNT = N>
		inline Vector<COUNT, T>& SRK_CALL cast(size_t start) {
			return *(Vector<COUNT, T>*)(data + start);
		}

		template<size_t COUNT = N>
		inline const Vector<COUNT, T>& SRK_CALL cast(size_t start) const {
			return *(const Vector<COUNT, T>*)(data + start);
		}

		template<typename Ret = T>
		inline constexpr Ret SRK_CALL getMultiplies() const {
			return Math::mul<N, T, Ret>(data);
		}

		template<typename In, typename Ret = decltype((*(T*)0) + (*(In*)0))>
		inline Ret SRK_CALL dot(const Vector<N, In>& v) const {
			return Math::dot<N, T, In, Ret>(data, v.data);
		}

		template<typename In1, typename In2, typename Ret>
		inline void SRK_CALL lerp(const Vector<N, In1>& to, const In2& t, Vector<N, Ret>& dst) const {
			Math::lerp(data, to.data, t, dst.data);
		}

		static const Vector ZERO;
		static const Vector ONE;

		Data data;
	};

	template<size_t N, typename T> const Vector<N, T> Vector<N, T>::ZERO = Vector<N, T>(Vector<N, T>::All<T>(Math::ZERO<T>));
	template<size_t N, typename T> const Vector<N, T> Vector<N, T>::ONE = Vector<N, T>(Vector<N, T>::All<T>(Math::ONE<T>));

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> SRK_CALL operator+(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
		v += v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> SRK_CALL operator+(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
			v += v2;
			return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) + (*(T2*)0))> SRK_CALL operator+(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) + (*(T2*)0))> v(v1);
		v += v2;
		return v;
	}

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> SRK_CALL operator-(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> SRK_CALL operator-(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) - (*(T2*)0))> SRK_CALL operator-(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) - (*(T2*)0))> v(v1);
		v -= v2;
		return v;
	}

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> SRK_CALL operator*(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> SRK_CALL operator*(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) * (*(T2*)0))> SRK_CALL operator*(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) * (*(T2*)0))> v(v1);
		v *= v2;
		return v;
	}

	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> SRK_CALL operator/(const Vector<N, T1>& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> SRK_CALL operator/(const Vector<N, T1>& v1, const T2& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}
	template<size_t N, typename T1, typename T2>
	inline constexpr Vector<N, decltype((*(T1*)0) / (*(T2*)0))> SRK_CALL operator/(const T1& v1, const Vector<N, T2>& v2) {
		Vector<N, decltype((*(T1*)0) / (*(T2*)0))> v(v1);
		v /= v2;
		return v;
	}
	template<size_t N, std::integral T1, std::integral T2>
	inline constexpr Vector<N, decltype((*(T1*)0) << (*(T2*)0))> SRK_CALL operator<<(const Vector<N, T1>& v1, T2 v2) {
		Vector<N, decltype((*(T1*)0) << (*(T2*)0))> v(v1);
		v <<= v2;
		return v;
	}
	template<size_t N, std::integral T1, std::integral T2>
	inline constexpr Vector<N, decltype((*(T1*)0) >> (*(T2*)0))> SRK_CALL operator>>(const Vector<N, T1>& v1, T2 v2) {
		Vector<N, decltype((*(T1*)0) >> (*(T2*)0))> v(v1);
		v >>= v2;
		return v;
	}

	template<typename T> using Vec1 = Vector<1, T>;
	using Vec1f32 = Vec1<float32_t>;
	using Vec1f64 = Vec1<float64_t>;
	using Vec1i8 = Vec1<int8_t>;
	using Vec1ui8 = Vec1<uint8_t>;
	using Vec1i16 = Vec1<int16_t>;
	using Vec1ui16 = Vec1<uint16_t>;
	using Vec1i32 = Vec1<int32_t>;
	using Vec1ui32 = Vec1<uint32_t>;
	using Vec1i64 = Vec1<int64_t>;
	using Vec1ui64 = Vec1<uint64_t>;
	template<typename T> using Vec2 = Vector<2, T>;
	using Vec2f32 = Vec2<float32_t>;
	using Vec2f64 = Vec2<float64_t>;
	using Vec2i8 = Vec2<int8_t>;
	using Vec2ui8 = Vec2<uint8_t>;
	using Vec2i16 = Vec2<int16_t>;
	using Vec2ui16 = Vec2<uint16_t>;
	using Vec2i32 = Vec2<int32_t>;
	using Vec2ui32 = Vec2<uint32_t>;
	using Vec2i64 = Vec2<int64_t>;
	using Vec2ui64 = Vec2<uint64_t>;
	template<typename T> using Vec3 = Vector<3, T>;
	using Vec3f32 = Vec3<float32_t>;
	using Vec3f64 = Vec3<float64_t>;
	using Vec3i8 = Vec3<int8_t>;
	using Vec3ui8 = Vec3<uint8_t>;
	using Vec3i16 = Vec3<int16_t>;
	using Vec3ui16 = Vec3<uint16_t>;
	using Vec3i32 = Vec3<int32_t>;
	using Vec3ui32 = Vec3<uint32_t>;
	using Vec3i64 = Vec3<int64_t>;
	using Vec3ui64 = Vec3<uint64_t>;
	template<typename T> using Vec4 = Vector<4, T>;
	using Vec4f32 = Vec4<float32_t>;
	using Vec4f64 = Vec4<float64_t>;
	using Vec4i8 = Vec4<int8_t>;
	using Vec4ui8 = Vec4<uint8_t>;
	using Vec4i16 = Vec4<int16_t>;
	using Vec4ui16 = Vec4<uint16_t>;
	using Vec4i32 = Vec4<int32_t>;
	using Vec4ui32 = Vec4<uint32_t>;
	using Vec4i64 = Vec4<int64_t>;
	using Vec4ui64 = Vec4<uint64_t>;
}