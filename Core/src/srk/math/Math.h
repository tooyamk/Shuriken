#pragma once

#include "srk/Global.h"
#include <cmath>

namespace srk {
	class SRK_CORE_DLL Math {
	public:
		template<Arithmetic T> inline static constexpr T ZERO = 0;
		template<Arithmetic T> inline static constexpr T ONE = 1;
		template<Arithmetic T> inline static constexpr T TWO = 2;
		template<Arithmetic T> inline static constexpr T NEGATIVE_ONE = -1;
		template<std::floating_point T> inline static constexpr T ONE_HALF = T(.5);
		template<std::floating_point T> inline static constexpr T TENTH = T(.1);
		template<std::floating_point T> inline static constexpr T TWENTIETH = T(.05);
		template<std::floating_point T> inline static constexpr T FORTIETH = T(.025);
		template<std::floating_point T> inline static constexpr T HUNDREDTH = T(.01);

		template<std::floating_point T> inline static constexpr T TOLERANCE = T(2e-37);
		template<std::floating_point T> inline static constexpr T E = T(2.718281828459045);
		template<std::floating_point T> inline static constexpr T PI = T(3.14159265358979323846);
		template<std::floating_point T> inline static constexpr T PI_2 = PI<T> * T(.5);
		template<std::floating_point T> inline static constexpr T PI_4 = PI<T> * T(.25);
		template<std::floating_point T> inline static constexpr T PI_6 = PI<T> / T(6.);
		template<std::floating_point T> inline static constexpr T PI_8 = PI<T> * T(.125);
		template<std::floating_point T> inline static constexpr T PI2 = PI<T> * T(2.);
		template<std::floating_point T> inline static constexpr T INV_PI = T(1.) / PI<T>;
		template<std::floating_point T> inline static constexpr T SQRT2 = T(1.4142135623730951);
		template<std::floating_point T> inline static constexpr T SQRT3 = T(0.5773502691896257);
		template<std::floating_point T> inline static constexpr T DEG = T(180.) / PI<T>;
		template<std::floating_point T> inline static constexpr T RAD = PI<T> / T(180.);

		template<typename In1, typename In2>
		inline static bool SRK_CALL isEqual(const In1& v1, const In2& v2) {
			return v1 == v2;
		}
		template<size_t N, typename In1, typename In2>
		inline static bool SRK_CALL isEqual(const In1(&v)[N], const In2& value) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v[i] != value) return false;
			}
			return true;
		}
		template<size_t N, typename In1, typename In2>
		inline static bool SRK_CALL isEqual(const In1(&v1)[N], const In2(&v2)[N]) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v1[i] != v2[i]) return false;
			}
			return true;
		}
		template<typename In1, typename In2, typename In3>
		inline static bool SRK_CALL isEqual(const In1& v1, const In2& v2, const In3& tolerance) {
			return (v1 < v2 ? v2 - v1 : v1 - v2) <= tolerance;
		}
		template<size_t N, typename In1, typename In2, typename In3>
		inline static bool SRK_CALL isEqual(const In1(&v)[N], const In2& value, const In3& tolerance) {
			for (decltype(N) i = 0; i < N; ++i) {
				if ((v[i] < value ? value - v[i] : v[i] - value) > tolerance) return false;
			}
			return true;
		}
		template<size_t N, typename In1, typename In2, typename In3>
		inline static bool SRK_CALL isEqual(const In1(&v1)[N], const In2(&v2)[N], const In3& tolerance) {
			for (decltype(N) i = 0; i < N; ++i) {
				if ((v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i]) > tolerance) return false;
			}
			return true;
		}

		template<typename In1, typename In2, typename In3, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static constexpr Out SRK_CALL clamp(const In1& v, const In2& min, const In3& max) {
			return v < min ? min : (v > max ? max : v);
		}

		template<ScopedEnum T>
		inline static constexpr T SRK_CALL clamp(T v, T min, T max) {
			return v < min ? min : (v > max ? max : v);
		}

		template<size_t N, typename In1, typename In2, typename In3, typename Out>
		inline static constexpr void SRK_CALL clamp(const In1(&v)[N], const In2& min, const In3& max, Out(&dst)[N]) {
			Out tmp[N];
			for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i] < min ? min : (v[i] > max ? max : v[i]);
			for (decltype(N) i = 0; i < N; ++i) dst[i] = tmp[i];
		}

		template<size_t N, typename In1, typename In2, typename In3>
		inline static constexpr void SRK_CALL clamp(In1(&v)[N], const In2& min, const In3& max) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v[i] < min) {
					v[i] = min;
				} else if (v[i] > max) {
					v[i] = max;
				}
			}
		}

		template<size_t N, typename In1, typename In2, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static Out SRK_CALL dot(const In1(&v1)[N], const In2(&v2)[N]) {
			Out rst = ZERO<Out>;
			for (decltype(N) i = 0; i < N; ++i) rst += v1[i] * v2[i];
			return rst;
		}

		template<typename In1, typename In2, typename Out>
		inline static void SRK_CALL cross(const In1(&v1)[3], const In2(&v2)[3], Out(&dst)[3]) {
			Out x = (v1[1] * v2[2]) - (v1[2] * v2[1]);
			Out y = (v1[2] * v2[0]) - (v1[0] * v2[2]);
			Out z = (v1[0] * v2[1]) - (v1[1] * v2[0]);

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}

		template<size_t N, typename In1, typename In2, typename In3, typename Out>
		inline static void SRK_CALL lerp(const In1(&from)[N], const In2(&to)[N], const In3 t, Out(&dst)[N]) {
			Out tmp[N];
			for (decltype(N) i = 0; i < N; ++i) tmp[i] = from[i] + (to[i] - from[i]) * t;
			for (decltype(N) i = 0; i < N; ++i) dst[i] = tmp[i];
		}

		inline static void SRK_CALL appendQuat(const float32_t(&lhs)[4], const float32_t(&rhs)[4], float32_t(&dst)[4]) {
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
		inline static void SRK_CALL quatRotate(const T(&q)[4], const T(&p)[3], float32_t(&dst)[3]) {
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

		static void SRK_CALL transposeMat(const float32_t(&m)[3][4], float32_t(&dst)[4][4]);
		static void SRK_CALL transposeMat(const float32_t(&m)[4][4], float32_t(&dst)[4][4]);

		static void SRK_CALL appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[3][4], float32_t(&dst)[3][4]);
		static void SRK_CALL appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[3][4], float32_t(&dst)[4][4]);
		static void SRK_CALL appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[4][4], float32_t(&dst)[3][4]);
		static void SRK_CALL appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[4][4], float32_t(&dst)[4][4]);
		static void SRK_CALL appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[3][4], float32_t(&dst)[3][4]);
		static void SRK_CALL appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[3][4], float32_t(&dst)[4][4]);
		static void SRK_CALL appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[4][4], float32_t(&dst)[3][4]);
		static void SRK_CALL appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[4][4], float32_t(&dst)[4][4]);

		static bool SRK_CALL invertMat(const float32_t(&m)[3][4], float32_t(&dst)[3][4]);
		static bool SRK_CALL invertMat(const float32_t(&m)[4][4], float32_t(&dst)[4][4]);

		inline static void SRK_CALL matTransformPoint(const float32_t(&m)[3][4], const float32_t(&p)[3], float32_t(&dst)[3]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}

		inline static void SRK_CALL matTransformPoint(const float32_t(&m)[4][4], const float32_t(&p)[4], float32_t(&dst)[4]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + p[3] * m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + p[3] * m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + p[3] * m[2][3];
			auto w = p[0] * m[3][0] + p[1] * m[3][1] + p[2] * m[3][2] + p[3] * m[3][3];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = w;
		}

		inline static void SRK_CALL matTransformPoint(const float32_t(&m)[4][4], const float32_t(&p)[3], float32_t(&dst)[4]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];
			auto w = p[0] * m[3][0] + p[1] * m[3][1] + p[2] * m[3][2] + m[3][3];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = w;
		}

		inline static void SRK_CALL matTransformPoint(const float32_t(&m)[4][4], const float32_t(&p)[3], float32_t(&dst)[3]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];
			auto w = p[0] * m[3][0] + p[1] * m[3][1] + p[2] * m[3][2] + m[3][3];

			dst[0] = x / w;
			dst[1] = y / w;
			dst[2] = z / w;
		}

		template<typename T>
		inline static constexpr FloatingPointType<T> SRK_CALL deg(const T& rad) {
			return rad * DEG<T>;
		}

		template<typename T>
		inline static constexpr FloatingPointType<T> SRK_CALL rad(const T& deg) {
			return deg * RAD<T>;
		}

		template<size_t N, typename In, typename Out = In>
		inline static constexpr Out SRK_CALL multiplies(const In(&v)[N]) {
			if constexpr (N == 0) {
				return Out(0);
			} else {
				Out m = v[0];
				for (decltype(N) i = 1; i < N; ++i) m *= v[i];
				return m;
			}
		}

		template<size_t N, typename In, typename Out = In>
		inline static Out SRK_CALL distanceSq(const In(&v1)[N], const In(&v2)[N]) {
			Out sq = 0;
			for (decltype(N) i = 0; i < N; ++i) {
				Out d = v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}
		template<size_t N, typename In1, typename In2, typename Out = decltype((*(In1*)0) + (*(In2*)0))>
		inline static Out SRK_CALL distanceSq(const FloatingPointType<In1>(&v1)[N], const FloatingPointType<In2>(&v2)[N]) {
			Out sq = 0;
			for (decltype(N) i = 0; i < N; ++i) {
				Out d = v1[i] - v2[i];
				sq += d * d;
			}
			return sq;
		}

		template<size_t N, typename In, typename Out>
		static void SRK_CALL normalize(const In(&v)[N], Out(&dst)[N]) {
			if constexpr (sizeof(In) >= sizeof(Out)) {
				if (auto n = dot<N, In, In, Out>(v, v); !isEqual(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
					n = std::sqrt(n);
					if (n > TOLERANCE<decltype(n)>) {
						n = ONE<decltype(n)> / n;

						if ((void*)&v >= (void*)&dst) {
							for (decltype(N) i = 0; i < N; ++i) dst[i] = v[i] * n;
						} else {
							for (decltype(N) i = N - 1; i < N; --i) dst[i] = v[i] * n;
						}
					} else {
						if ((void*)&v >= (void*)&dst) {
							for (decltype(N) i = 0; i < N; ++i) dst[i] = v[i];
						} else {
							for (decltype(N) i = N - 1; i < N; --i) dst[i] = v[i];
						}
					}
				} else {
					if ((void*)&v >= (void*)&dst) {
						for (decltype(N) i = 0; i < N; ++i) dst[i] = v[i];
					} else {
						for (decltype(N) i = N - 1; i < N; --i) dst[i] = v[i];
					}
				}
			} else {
				Out tmp[N];
				if (auto n = dot<N, In, In, Out>(v, v); !isEqual(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
					n = std::sqrt(n);
					if (n > TOLERANCE<decltype(n)>) {
						n = ONE<decltype(n)> / n;

						for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i] * n;
					} else {
						for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i];
					}
				} else {
					for (decltype(N) i = 0; i < N; ++i) tmp[i] = v[i];
				}
				memcpy(dst, tmp, N * sizeof(tmp));
			}
		}

		template<size_t N, typename T>
		static void SRK_CALL normalize(T(&val)[N]) {
			if (auto n = dot(val, val); !isEqual(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
				n = std::sqrt(n);
				if (n > TOLERANCE<decltype(n)>) {
					n = ONE<decltype(n)> / n;

					for (decltype(N) i = 0; i < N; ++i) val[i] *= n;
				}
			}
		}

		template<size_t N, typename In1, typename In2, std::floating_point Out = decltype((*(In1*)0) + (*(In2*)0))>
		static Out SRK_CALL angle(const In1(&v1)[N], const In2(&v2)[N]) {
			Out n1[N], n2[N];
			normalize(v1, n1);
			normalize(v2, n2);
			return std::acos(clamp(dot(n1, n2), -ONE<Out>, ONE<Out>));
		}

		template<size_t N, typename In1, typename In2, std::floating_point Out = decltype((*(In1*)0) + (*(In2*)0))>
		requires (N >= 2 && N <= 3)
		static Out SRK_CALL singedAngle(const In1(&v1)[N], const In2(&v2)[N]) {
			Out n1[N], n2[N];
			normalize(v1, n1);
			normalize(v2, n2);
			
			auto a = std::acos(clamp(dot(n1, n2), -ONE<Out>, ONE<Out>));
			return (n1[0] * n2[1]) - (n1[1] * n2[0]) < ZERO<Out> ? -a : a;
		}

		static void SRK_CALL slerp(const float32_t(&from)[4], const float32_t(&to)[4], float32_t t, float32_t(&dst)[4]);

		template<typename In1, typename In2, typename In3, std::floating_point Out>
		static void SRK_CALL slerp(const In1(&v1)[3], const In2(&v2)[3], const In3& t, Out(&out)[3]) {
			auto a = std::sqrt(dot<3, In1, In1, Out>(v1, v1));
			auto b = std::sqrt(dot<3, In2, In2, Out>(v2, v2));

			Out nrmA[3], nrmB[3];
			for (uint32_t i = 0; i < 3; ++i) {
				nrmA[i] = v1[i] / a;
				nrmB[i] = v2[i] / b;
			}

			auto d = clamp(dot(nrmA, nrmB), -ONE<Out>, ONE<Out>);
			auto theta = std::acos(d) * t;
			Out tmp[3];
			for (uint32_t i = 0; i < 3; ++i) tmp[i] = nrmB[i] - nrmA[i] * d;
			normalize(tmp, ONE<float32_t>);

			auto t1 = a + (b - a) * t;
			auto s = std::sin(theta);
			auto c = std::cos(theta);
			for (uint32_t i = 0; i < 3; ++i) out[i] = nrmA[i] * c + tmp[i] * s;
		}

		inline static constexpr uint32_t SRK_CALL potLog2(uint32_t pow) {
			return ((pow >> 23) & 0xFF) - 127;
		}
	};
}