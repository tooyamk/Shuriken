#pragma once

#include "base/LowLevel.h"
#include <cmath>

namespace aurora {
	class AE_DLL Math {
	public:
		AE_DECLA_CANNOT_INSTANTIATE(Math);
		
		template<typename T> using NumberType = typename std::enable_if_t<std::is_arithmetic_v<T>, T>;
		template<typename T> using FloatType = typename std::enable_if_t<std::is_floating_point_v<T>, T>;
		template<typename T> using IntType = typename std::enable_if_t<std::is_integral_v<T>, T>;
		template<typename T> using UIntType = typename std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>;

		template<typename T> inline static const T NUMBER_O = 0;
		template<typename T> inline static const T NUMBER_1 = 1;
		template<typename T> inline static const FloatType<T> TOLERANCE = T(2e-37);
		template<typename T> inline static const FloatType<T> PI = T(3.14159265358979323846);
		template<typename T> inline static const FloatType<T> PI_2 = PI<T> * T(.5);
		template<typename T> inline static const FloatType<T> PI_4 = PI<T> * T(.25);
		template<typename T> inline static const FloatType<T> PI2 = PI<T> * T(2.);
		template<typename T> inline static const FloatType<T> DEG = T(180.) / PI<T>;
		template<typename T> inline static const FloatType<T> RAD = PI<T> / T(180.);

		template<typename In>
		inline static bool AE_CALL isEqual(const In v1, const In v2) {
			return v1 == v2;
		}
		template<ui32 N, typename In>
		inline static bool AE_CALL isEqual(const In(&v)[N], const In value) {
			for (ui32 i = 0; i < N; ++i) {
				if (v[i] != value) return false;
			}
			return true;
		}
		template<ui32 N, typename In>
		inline static bool AE_CALL isEqual(const In(&v1)[N], const In(&v2)[N]) {
			for (ui32 i = 0; i < N; ++i) {
				if (v1[i] != v2[i]) return false;
			}
			return true;
		}
		template<typename In>
		inline static bool AE_CALL isEqual(const In v1, const In v2, const In tolerance) {
			return (v1 < v2 ? v2 - v1 : v1 - v2) <= tolerance;
		}
		template<ui32 N, typename In>
		inline static bool AE_CALL isEqual(const In(&v)[N], const In value, const In tolerance) {
			for (ui32 i = 0; i < N; ++i) {
				if ((v[i] < value ? value - v[i] : v[i] - value) > tolerance) return false;
			}
			return true;
		}
		template<ui32 N, typename In>
		inline static bool AE_CALL isEqual(const In(&v1)[N], const In(&v2)[N], const In tolerance) {
			for (ui32 i = 0; i < N; ++i) {
				if ((v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i]) > tolerance) return false;
			}
			return true;
		}

		template<ui32 N, typename In, typename Out>
		inline static Out AE_CALL dot(const In(&v1)[N], const In(&v2)[N]) {
			Out rst = 0;
			for (ui32 i = 0; i < N; ++i) rst += v1[i] * v2[i];
			return rst;
		}

		template<typename In, typename Out>
		inline static void AE_CALL cross(const In(&v1)[3], const In(&v2)[3], Out(&dst)[3]) {
			Out x = (v1[1] * v2[2]) - (v1[2] * v2[1]);
			Out y = (v1[2] * v2[0]) - (v1[0] * v2[2]);
			Out z = (v1[0] * v2[1]) - (v1[1] * v2[0]);

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}

		template<ui32 N, typename In, typename Out>
		inline static void AE_CALL lerp(const In(&from)[N], const In(&to)[N], f32 t, Out(&dst)[N]) {
			Out tmp[N];
			for (ui32 i = 0; i < N; ++i) tmp[i] = from[i] + (to[i] - from[i]) * t;
			for (ui32 i = 0; i < N; ++i) dst[i] = tmp[i];
		}

		static void AE_CALL slerpQuat(const f32* from, const f32* to, f32 t, f32* dst);
		inline static void AE_CALL appendQuat(const f32* lhs, const f32* rhs, f32* dst);

		template<typename T>
		inline static void AE_CALL quatRotate(const T(&q)[4], const T(&p)[3], f32(&dst)[3]) {
			auto w = -p[0] * q[0] - p[1] * q[1] - p[2] * q[2];
			auto x = q[3] * p[0] + q[1] * p[2] - q[2] * p[1];
			auto y = q[3] * p[1] - q[0] * p[2] + q[2] * p[0];
			auto z = q[3] * p[2] + q[0] * p[1] - q[1] * p[0];

			auto dx = -w * q[0] + x * q[3] - y * q[2] + z * q[1];
			auto dy = -w * q[1] + x * q[2] + y * q[3] - z * q[0];
			dst[0] = dx;
			dst[1] = dy;
			dst[2] = -w * q[2] - x * q[1] + y * q[0] + z * q[3];
		}

		static void AE_CALL transposeMat(const f32(&m)[3][4], f32(&dst)[4][4]);
		static void AE_CALL transposeMat(const f32(&m)[4][4], f32(&dst)[4][4]);

		static void AE_CALL appendMat(const f32(&lhs)[3][4], const f32(&rhs)[3][4], f32(&dst)[3][4]);
		static void AE_CALL appendMat(const f32(&lhs)[3][4], const f32(&rhs)[3][4], f32(&dst)[4][4]);
		static void AE_CALL appendMat(const f32(&lhs)[3][4], const f32(&rhs)[4][4], f32(&dst)[3][4]);
		static void AE_CALL appendMat(const f32(&lhs)[3][4], const f32(&rhs)[4][4], f32(&dst)[4][4]);
		static void AE_CALL appendMat(const f32(&lhs)[4][4], const f32(&rhs)[3][4], f32(&dst)[3][4]);
		static void AE_CALL appendMat(const f32(&lhs)[4][4], const f32(&rhs)[3][4], f32(&dst)[4][4]);
		static void AE_CALL appendMat(const f32(&lhs)[4][4], const f32(&rhs)[4][4], f32(&dst)[3][4]);
		static void AE_CALL appendMat(const f32(&lhs)[4][4], const f32(&rhs)[4][4], f32(&dst)[4][4]);

		static bool AE_CALL invertMat(const f32(&m)[3][4], f32(&dst)[3][4]);
		static bool AE_CALL invertMat(const f32(&m)[4][4], f32(&dst)[4][4]);

		inline static void AE_CALL matTransformPoint(const f32(&m)[3][4], const f32(&p)[3], f32(&dst)[3]);

		template<typename T>
		inline static constexpr FloatType<T> AE_CALL deg(T rad) {
			return rad * DEG<T>;
		}

		template<typename T>
		inline static constexpr FloatType<T> AE_CALL rad(T deg) {
			return deg * RAD<T>;
		}

		template<ui32 N, typename In, typename Out = In>
		inline static Out AE_CALL distanceSq(const In(&v1)[N], const In(&v2)[N]) {
			Out sq = 0;
			for (ui32 i = 0; i < N; ++i) {
				Out d = v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}
		template<ui32 N, typename In, typename Out = In>
		inline static Out AE_CALL distanceSq(const FloatType<In>(&v1)[N], const FloatType<In>(&v2)[N]) {
			Out sq = 0;
			for (ui32 i = 0; i < N; ++i) {
				In d = v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}

		template<ui32 N, typename In, typename Out = In>
		static void AE_CALL normalize(const In(&v)[N], Out(&dst)[N]) {
			auto n = dot<N, In, f32>(v, v);
			if (!isEqual<In>(n, 1, TOLERANCE<decltype(n)>)) {
				n = std::sqrt(n);
				if (n > TOLERANCE<decltype(n)>) {
					n = NUMBER_1<decltype(n)> / n;

					for (ui32 i = 0; i < N; ++i) dst[i] *= n;
				} else if (v != dst) {
					for (ui32 i = 0; i < N; ++i) dst[i] = v[i];
				}
			} else if (v != dst) {
				for (ui32 i = 0; i < N; ++i) dst[i] = v[i];
			}
		}

		template<ui32 N, typename In, typename Out = In>
		static Out AE_CALL angleBetween(const In(&v1)[N], const In(&v2)[N]) {
			In n1[N], n2[N];
			normalize<N, In, In>(v1, n1);
			normalize<N, In, In>(v2, n2);
			Out a = dot<N, In, Out>(n1, n2);
			if (a > NUMBER_1<Out>) {
				a = NUMBER_1<Out>;
			} else if (a < -NUMBER_1<Out>) {
				a = -NUMBER_1<Out>;
			}
			return std::acos(a);
		}

		inline static bool AE_CALL isPOT(ui32 n);
	};
}

#include "Math.inl"