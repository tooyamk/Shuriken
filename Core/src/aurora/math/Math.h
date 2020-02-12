#pragma once

#include "aurora/Global.h"
#include <cmath>

namespace aurora {
	class AE_DLL Math {
	public:
		AE_DECLARE_CANNOT_INSTANTIATE(Math);
		
		template<typename T> using NumberType = typename std::enable_if_t<std::is_arithmetic_v<T>, T>;
		template<typename T> using FloatType = typename std::enable_if_t<std::is_floating_point_v<T>, T>;
		template<typename T> using IntType = typename std::enable_if_t<std::is_integral_v<T>, T>;
		template<typename T> using UIntType = typename std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>;

		template<typename T> inline static constexpr NumberType<T> NUMBER_0 = 0;
		template<typename T> inline static constexpr NumberType<T> NUMBER_1 = 1;
		template<typename T> inline static constexpr FloatType<T> TOLERANCE = T(2e-37);
		template<typename T> inline static constexpr FloatType<T> PI = T(3.14159265358979323846);
		template<typename T> inline static constexpr FloatType<T> PI_2 = PI<T> * T(.5);
		template<typename T> inline static constexpr FloatType<T> PI_4 = PI<T> * T(.25);
		template<typename T> inline static constexpr FloatType<T> PI2 = PI<T> * T(2.);
		template<typename T> inline static constexpr FloatType<T> DEG = T(180.) / PI<T>;
		template<typename T> inline static constexpr FloatType<T> RAD = PI<T> / T(180.);

		template<typename In1, typename In2>
		inline static bool AE_CALL isEqual(const In1& v1, const In2& v2) {
			return v1 == v2;
		}
		template<uint32_t N, typename In1, typename In2>
		inline static bool AE_CALL isEqual(const In1(&v)[N], const In2& value) {
			for (uint32_t i = 0; i < N; ++i) {
				if (v[i] != value) return false;
			}
			return true;
		}
		template<uint32_t N, typename In1, typename In2>
		inline static bool AE_CALL isEqual(const In1(&v1)[N], const In2(&v2)[N]) {
			for (uint32_t i = 0; i < N; ++i) {
				if (v1[i] != v2[i]) return false;
			}
			return true;
		}
		template<typename In1, typename In2, typename In3>
		inline static bool AE_CALL isEqual(const In1& v1, const In2& v2, const In3& tolerance) {
			return (v1 < v2 ? v2 - v1 : v1 - v2) <= tolerance;
		}
		template<uint32_t N, typename In1, typename In2, typename In3>
		inline static bool AE_CALL isEqual(const In1(&v)[N], const In2& value, const In3& tolerance) {
			for (uint32_t i = 0; i < N; ++i) {
				if ((v[i] < value ? value - v[i] : v[i] - value) > tolerance) return false;
			}
			return true;
		}
		template<uint32_t N, typename In1, typename In2, typename In3>
		inline static bool AE_CALL isEqual(const In1(&v1)[N], const In2(&v2)[N], const In3& tolerance) {
			for (uint32_t i = 0; i < N; ++i) {
				if ((v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i]) > tolerance) return false;
			}
			return true;
		}

		template<typename In1, typename In2, typename In3, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static constexpr Out AE_CALL clamp(const In1& v, const In2& min, const In3& max) {
			return v < min ? min : (v > max ? max : v);
		}

		template<uint32_t N, typename In1, typename In2, typename In3, typename Out>
		inline static constexpr void AE_CALL clamp(const In1(&v)[N], const In2& min, const In3& max, Out(&dst)[N]) {
			Out tmp[N];
			for (uint32_t i = 0; i < N; ++i) tmp[i] = v[i] < min ? min : (v[i] > max ? max : v[i]);
			for (uint32_t i = 0; i < N; ++i) dst[i] = tmp[i];
		}

		template<uint32_t N, typename In1, typename In2, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static Out AE_CALL dot(const In1(&v1)[N], const In2(&v2)[N]) {
			Out rst(0);
			for (uint32_t i = 0; i < N; ++i) rst += v1[i] * v2[i];
			return rst;
		}

		template<typename In1, typename In2, typename Out>
		inline static void AE_CALL cross(const In1(&v1)[3], const In2(&v2)[3], Out(&dst)[3]) {
			Out x = (v1[1] * v2[2]) - (v1[2] * v2[1]);
			Out y = (v1[2] * v2[0]) - (v1[0] * v2[2]);
			Out z = (v1[0] * v2[1]) - (v1[1] * v2[0]);

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}

		template<uint32_t N, typename In1, typename In2, typename In3, typename Out>
		inline static void AE_CALL lerp(const In1(&from)[N], const In2(&to)[N], const In3 t, Out(&dst)[N]) {
			Out tmp[N];
			for (uint32_t i = 0; i < N; ++i) tmp[i] = from[i] + (to[i] - from[i]) * t;
			for (uint32_t i = 0; i < N; ++i) dst[i] = tmp[i];
		}

		static void AE_CALL slerpQuat(const f32* from, const f32* to, f32 t, f32* dst);
		inline static void AE_CALL appendQuat(const f32(&lhs)[4], const f32(&rhs)[4], f32(&dst)[4]) {
			auto w = lhs[3] * rhs[3] - lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2];
			auto x = lhs[0] * rhs[3] + lhs[3] * rhs[0] + lhs[2] * rhs[1] - lhs[1] * rhs[2];
			auto y = lhs[1] * rhs[3] + lhs[3] * rhs[1] + lhs[0] * rhs[2] - lhs[2] * rhs[0];
			auto z = lhs[2] * rhs[3] + lhs[3] * rhs[2] + lhs[1] * rhs[0] - lhs[0] * rhs[1];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = w;
		}

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

		inline static void AE_CALL matTransformPoint(const f32(&m)[3][4], const f32(&p)[3], f32(&dst)[3]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}

		template<typename T>
		inline static constexpr FloatType<T> AE_CALL deg(const T& rad) {
			return rad * DEG<T>;
		}

		template<typename T>
		inline static constexpr FloatType<T> AE_CALL rad(const T& deg) {
			return deg * RAD<T>;
		}

		template<uint32_t N, typename In, typename Out = In>
		inline static constexpr Out AE_CALL multiplies(const In(&v)[N]) {
			if constexpr (N == 0) {
				return Out(0);
			} else {
				Out m = v[0];
				for (uint32_t i = 1; i < N; ++i) m *= v[i];
				return m;
			}
		}

		template<uint32_t N, typename In, typename Out = In>
		inline static Out AE_CALL distanceSq(const In(&v1)[N], const In(&v2)[N]) {
			Out sq = 0;
			for (uint32_t i = 0; i < N; ++i) {
				Out d = v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}
		template<uint32_t N, typename In1, typename In2, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static Out AE_CALL distanceSq(const FloatType<In1>(&v1)[N], const FloatType<In2>(&v2)[N]) {
			Out sq = 0;
			for (uint32_t i = 0; i < N; ++i) {
				In d = v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}

		template<uint32_t N, typename In, typename Out = f32>
		static void AE_CALL normalize(const In(&v)[N], Out(&dst)[N]) {
			if constexpr (sizeof(In) >= sizeof(Out)) {
				if (auto n = dot<N, In, In, f32>(v, v); !isEqual(n, 1, TOLERANCE<decltype(n)>)) {
					n = std::sqrt(n);
					if (n > TOLERANCE<decltype(n)>) {
						n = NUMBER_1<decltype(n)> / n;

						if ((void*)&v >= (void*)&dst) {
							for (uint32_t i = 0; i < N; ++i) dst[i] = v[i] * n;
						} else {
							for (uint32_t i = N - 1; i < N; --i) dst[i] = v[i] * n;
						}
					} else {
						if ((void*)&v >= (void*)&dst) {
							for (uint32_t i = 0; i < N; ++i) dst[i] = v[i];
						} else {
							for (uint32_t i = N - 1; i < N; --i) dst[i] = v[i];
						}
					}
				} else {
					if ((void*)&v >= (void*)&dst) {
						for (uint32_t i = 0; i < N; ++i) dst[i] = v[i];
					} else {
						for (uint32_t i = N - 1; i < N; --i) dst[i] = v[i];
					}
				}
			} else {
				Out tmp[N];
				if (auto n = dot<N, In, In, f32>(v, v); !isEqual(n, 1, TOLERANCE<decltype(n)>)) {
					n = std::sqrt(n);
					if (n > TOLERANCE<decltype(n)>) {
						n = NUMBER_1<decltype(n)> / n;

						for (uint32_t i = 0; i < N; ++i) tmp[i] = v[i] * n;
					} else {
						for (uint32_t i = 0; i < N; ++i) tmp[i] = v[i];
					}
				} else {
					for (uint32_t i = 0; i < N; ++i) tmp[i] = v[i];
				}
				memcpy(dst, tmp, N * sizeof(tmp));
			}
		}

		template<uint32_t N, typename In1, typename In2, typename Out = f32>
		static Out AE_CALL angleBetween(const In1(&v1)[N], const In2(&v2)[N]) {
			Out n1[N];
			Out n2[N];
			normalize(v1, n1);
			normalize(v2, n2);
			Out a = dot<N, Out, Out, Out>(n1, n2);
			if (a > NUMBER_1<Out>) {
				a = NUMBER_1<Out>;
			} else if (a < -NUMBER_1<Out>) {
				a = -NUMBER_1<Out>;
			}
			return std::acos(a);
		}

		inline static bool AE_CALL isPOT(uint32_t n) {
			return n < 1 ? false : !(n & (n - 1));
		}
		inline static constexpr uint32_t AE_CALL potLog2(uint32_t pow) {
			auto f = (f32)pow;
			return ((((uint32_t&)f) >> 23) & 0xFF) - 127;
		}
	};
}