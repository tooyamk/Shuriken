#pragma once

#include "srk/Global.h"
#include <cmath>

namespace srk {
	class SRK_CORE_DLL Math {
	public:
		enum class Hint : uint8_t {
			NONE = 0,
			MEM_OVERLAP = 1 << 0,
			MATRIX_IDENTITY_VALUE = 1 << 1
		};

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

		template<size_t N, typename In1, typename In2, std::floating_point Out = decltype((*(In1*)0) + (*(In2*)0))>
		static Out SRK_CALL angle(const In1(&v1)[N], const In2(&v2)[N]) {
			Out n1[N], n2[N];
			normalize(v1, n1);
			normalize(v2, n2);
			return std::acos(clamp(dot(n1, n2), -ONE<Out>, ONE<Out>));
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

		template<size_t SC, size_t SR, std::floating_point ST, size_t DN, std::floating_point DT>
		static bool SRK_CALL transform(const ST(&m)[SC][SR], DT(&dst)[DN]) {
			if constexpr (SC < 3 && SR < 3) {
				return false;
			} else {
				constexpr auto n = std::min(DN, 4);

				if (auto tr = m[0][0] + m[1][1] + m[2][2]; tr > ZERO<ST>) {
					auto s = std::sqrt(tr + 1.f);
					if constexpr (n > 3) dst.w = s * ONE_HALF<ST>;
					s = ONE_HALF<ST> / s;
					if constexpr (n > 0) dst.x = (m[2][1] - m[1][2]) * s;
					if constexpr (n > 1) dst.y = (m[0][2] - m[2][0]) * s;
					if constexpr (n > 2) dst.z = (m[1][0] - m[0][1]) * s;
				} else if (m[1][1] > m[0][0]) {
					if (m[2][2] > m[1][1]) {//2
						auto s = std::sqrt(m[2][2] - m[0][0] - m[1][1] + ONE<ST>);
						if constexpr (n > 2) dst.z = s * ONE_HALF<ST>;
						s = ONE_HALF<ST> / s;
						if constexpr (n > 0) dst.x = (m[0][2] + m[2][0]) * s;
						if constexpr (n > 1) dst.y = (m[1][2] + m[2][1]) * s;
						if constexpr (n > 3) dst.w = (m[1][0] - m[0][1]) * s;
					} else {//1
						auto s = std::sqrt(m[1][1] - m[2][2] - m[0][0] + ONE<ST>);
						if constexpr (n > 1) dst.y = s * ONE_HALF<ST>;
						s = ONE_HALF<ST> / s;
						if constexpr (n > 0) dst.x = (m[0][1] + m[1][0]) * s;
						if constexpr (n > 2) dst.z = (m[2][1] + m[1][2]) * s;
						if constexpr (n > 3) dst.w = (m[0][2] - m[2][0]) * s;
					}
				} else if (m[2][2] > m[0][0]) {//2
					auto s = std::sqrt(m[2][2] - m[0][0] - m[1][1] + ONE<ST>);
					if constexpr (n > 2) dst.z = s * ONE_HALF<ST>;
					s = ONE_HALF<ST> / s;
					if constexpr (n > 0) dst.x = (m[0][2] + m[2][0]) * s;
					if constexpr (n > 1) dst.y = (m[1][2] + m[2][1]) * s;
					if constexpr (n > 3) dst.w = (m[1][0] - m[0][1]) * s;
				} else {//0
					auto s = std::sqrt(m[0][0] - m[1][1] - m[2][2] + ONE<ST>);
					if constexpr (n > 0) dst.x = s * ONE_HALF<ST>;
					s = ONE_HALF<ST> / s;
					if constexpr (n > 1) dst.y = (m[1][0] + m[0][1]) * s;
					if constexpr (n > 2) dst.z = (m[2][0] + m[0][2]) * s;
					if constexpr (n > 3) dst.w = (m[2][1] - m[1][2]) * s;
				}

				return true;
			}
		}

	private:
		template<size_t I, size_t DstBegin, size_t Range, size_t DstN, typename DstT, typename T, typename... Args>
		inline static void SRK_CALL _copy(DstT(&dst)[DstN], T&& val, Args&&... args) {
			if constexpr (I + DstBegin < std::min(DstBegin + Range, DstN)) {
				dst[I + DstBegin] = val;
				_copy<I + 1, DstBegin, Range>(dst, std::forward<Args>(args)...);
			}
		}

		template<size_t I, size_t DstBegin, size_t Range, size_t DstN, typename DstT>
		inline static void SRK_CALL _copy(DstT(&dst)[DstN]) {}

	public:
		template<size_t DstBegin, size_t Range, size_t DstN, typename DstT, std::convertible_to<DstT>... Args>
		inline static void SRK_CALL copy(DstT(&dst)[DstN], Args&&... args) {
			_copy<0, DstBegin, Range>(dst, std::forward<Args>(args)...);
		}

		template<size_t RangeBeginC, size_t RangeBeginR, size_t Range, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT, std::convertible_to<DstT>... Args>
		inline static void SRK_CALL copy(DstT(&dst)[DstTotalCs][DstTotalRs], Args&&... args) {
			copy<RangeBeginC * DstTotalRs + RangeBeginR, Range>((DstT(&)[DstTotalCs * DstTotalRs])dst, std::forward<Args>(args)...);
		}

	private:
		template<size_t CurC, size_t CurR, Hint Hints, size_t SrcRangeBeginC, size_t SrcRangeBeginR, size_t DstRangeBeginC, size_t DstRangeBeginR, size_t RangeCs, size_t RangeRs, size_t SrcTotalCs, size_t SrcTotalRs, std::floating_point SrcT, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL _copyInside(const SrcT(&src)[SrcTotalCs][SrcTotalRs], DstT(&dst)[DstTotalCs][DstTotalRs]) {
			using namespace srk::enum_operators;

			if constexpr (CurR < RangeRs) {
				if constexpr (CurR + DstRangeBeginR < DstTotalRs && CurC + DstRangeBeginC < DstTotalCs) {
					if constexpr (CurC + SrcRangeBeginC < SrcTotalCs && CurR + SrcRangeBeginR < SrcTotalRs) {
						dst[CurC + DstRangeBeginC][CurR + DstRangeBeginR] = src[CurC + SrcRangeBeginC][CurR + SrcRangeBeginR];
					} else if constexpr ((Hints & Hint::MATRIX_IDENTITY_VALUE) == Hint::MATRIX_IDENTITY_VALUE) {
						if constexpr (CurC + DstRangeBeginC == CurR + DstRangeBeginR) {
							dst[CurC + DstRangeBeginC][CurR + DstRangeBeginR] = ONE<DstT>;
						} else {
							dst[CurC + DstRangeBeginC][CurR + DstRangeBeginR] = ZERO<DstT>;
						}
					}
				}

				if constexpr (CurR + 1 == RangeRs) {
					if constexpr (CurC + 1 < RangeCs) {
						_copyInside<CurC + 1, 0, Hints, SrcRangeBeginC, SrcRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, DstTotalCs, DstTotalRs, DstT>(src, dst);
					}
				} else {
					_copyInside<CurC, CurR + 1, Hints, SrcRangeBeginC, SrcRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, DstTotalCs, DstTotalRs, DstT>(src, dst);
				}
			}
		}

		template<size_t CurC, size_t CurR, Hint Hints, size_t SrcRangeBeginC, size_t SrcRangeBeginR, size_t DstRangeBeginC, size_t DstRangeBeginR, size_t RangeCs, size_t RangeRs, size_t SrcTotalCs, size_t SrcTotalRs, std::floating_point SrcT, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL _copyOutside(const SrcT(&src)[SrcTotalCs][SrcTotalRs], DstT(&dst)[DstTotalCs][DstTotalRs]) {
			using namespace srk::enum_operators;

			if constexpr (CurR < DstTotalRs) {
				if constexpr (CurR < DstRangeBeginR || CurR >= DstRangeBeginR + RangeRs || CurC < DstRangeBeginC || CurC >= DstRangeBeginC + RangeCs) {
					if constexpr (CurR + DstRangeBeginR < DstTotalRs && CurC + DstRangeBeginC < DstTotalCs) {
						if constexpr (CurC + SrcRangeBeginC < SrcTotalCs && CurR + SrcRangeBeginR < SrcTotalRs) {
							dst[CurC + DstRangeBeginC][CurR + DstRangeBeginR] = src[CurC + SrcRangeBeginC][CurR + SrcRangeBeginR];
						} else if constexpr ((Hints & Hint::MATRIX_IDENTITY_VALUE) == Hint::MATRIX_IDENTITY_VALUE) {
							if constexpr (CurC + DstRangeBeginC == CurR + DstRangeBeginR) {
								dst[CurC + DstRangeBeginC][CurR + DstRangeBeginR] = ONE<DstT>;
							} else {
								dst[CurC + DstRangeBeginC][CurR + DstRangeBeginR] = ZERO<DstT>;
							}
						}
					}
				}

				if constexpr (CurR + 1 == DstTotalRs) {
					if constexpr (CurC + 1 < DstTotalCs) {
						_copyOutside<CurC + 1, 0, Hints, SrcRangeBeginC, SrcRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, DstTotalCs, DstTotalRs, DstT>(src, dst);
					}
				} else {
					_copyOutside<CurC, CurR + 1, Hints, SrcRangeBeginC, SrcRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, DstTotalCs, DstTotalRs, DstT>(src, dst);
				}
			}
		}

	public:
		template<Hint Hints = Hint::NONE, size_t SrcRangeBeginC, size_t SrcRangeBeginR, size_t DstRangeBeginC, size_t DstRangeBeginR, size_t RangeCs, size_t RangeRs, bool Inside, size_t SrcTotalCs, size_t SrcTotalRs, std::floating_point SrcT, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL copy(const SrcT(&src)[SrcTotalCs][SrcTotalRs], DstT(&dst)[DstTotalCs][DstTotalRs]) {
			using namespace srk::enum_operators;

			if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
				if constexpr (Inside) {
					constexpr auto nc = std::min(RangeCs, DstTotalCs);
					constexpr auto nr = std::min(RangeRs, DstTotalRs);
					DstT d[nc][nr];
					_copyInside<0, 0, Hints, SrcRangeBeginC, SrcRangeBeginR, 0, 0, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, nc, nr, DstT>(src, d);
					_copyInside<0, 0, Hints, 0, 0, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, nc, nr, DstT, DstTotalCs, DstTotalRs, DstT>(d, dst);
				} else {
					DstT d[DstTotalCs][DstTotalRs];
					_copyOutside<0, 0, Hints, SrcRangeBeginC, SrcRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, DstTotalCs, DstTotalRs, DstT>(src, d);
					_copyOutside<0, 0, Hints, DstRangeBeginC, DstRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, DstTotalCs, DstTotalRs, DstT, DstTotalCs, DstTotalRs, DstT>(d, dst);
				}
			} else {
				if constexpr (Inside) {
					_copyInside<0, 0, Hints, SrcRangeBeginC, SrcRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, DstTotalCs, DstTotalRs, DstT>(src, dst);
				} else {
					_copyOutside<0, 0, Hints, SrcRangeBeginC, SrcRangeBeginR, DstRangeBeginC, DstRangeBeginR, RangeCs, RangeRs, SrcTotalCs, SrcTotalRs, SrcT, DstTotalCs, DstTotalRs, DstT>(src, dst);
				}
			}
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

		template<size_t C, size_t R, std::floating_point T, typename RT, typename ST>
		requires ((std::same_as<RT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<RT>> && !std::is_const_v<std::remove_reference_t<RT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<RT>>> && std::rank_v<std::remove_cvref_t<RT>> == 2)) &&
			(std::same_as<ST, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<ST>> && !std::is_const_v<std::remove_reference_t<ST>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<ST>>> && std::rank_v<std::remove_cvref_t<ST>> == 1)))
		static bool SRK_CALL decomposition(const T(&m)[C][R], RT&& dstRot, ST&& dstScale) {
			if constexpr (C < 3 || R < 3) {
				return false;
			} else {
				T d[3][3];

				T xyz[3] = { m[0][0], m[1][0], m[2][0] };
				for (auto i = 0; i < 3; ++i) d[i][0] = xyz[i];

				auto dot = Math::dot(xyz, xyz);
				if (dot != ONE<T>) {
					if (dot = std::sqrt(dot); dot > TOLERANCE<T>) {
						dot = ONE<T> / dot;
						for (auto i = 0; i < 3; ++i) d[i][0] *= dot;
					}
				}

				for (auto i = 0; i < 3; ++i) xyz[i] = m[i][1];
				dot = d[0][0] * xyz[0] + d[1][0] * xyz[1] + d[2][0] * xyz[2];
				for (auto i = 0; i < 3; ++i) {
					xyz[i] -= d[i][0] * dot;
					d[i][1] = xyz[i];
				}

				dot = xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2];
				if (dot != ONE<T>) {
					if (dot = std::sqrt(dot); dot > TOLERANCE<T>) {
						dot = ONE<T> / dot;
						for (auto i = 0; i < 3; ++i) d[i][1] *= dot;
					}
				}

				for (auto i = 0; i < 3; ++i) xyz[i] = m[i][2];
				dot = d[0][0] * xyz[0] + d[1][0] * xyz[1] + d[2][0] * xyz[2];
				for (auto i = 0; i < 3; ++i) d[i][2] = xyz[i] - d[i][0] * dot;

				dot = d[0][1] * xyz[0] + d[1][1] * xyz[1] + d[2][1] * xyz[2];
				for (auto i = 0; i < 3; ++i) d[i][2] -= d[i][1] * dot;

				dot = d[0][2] * xyz[0] + d[1][2] * xyz[1] + d[2][2] * xyz[2];
				if (dot != ONE<T>) {
					if (dot = std::sqrt(dot); dot > TOLERANCE<T>) {
						dot = ONE<T> / dot;
						for (auto i = 0; i < 3; ++i) d[i][2] *= dot;
					}
				}

				dot = d[0][0] * d[1][1] * d[2][2] +
					d[0][1] * d[1][2] * d[2][0] +
					d[0][2] * d[1][0] * d[2][1] -
					d[0][2] * d[1][1] * d[2][0] -
					d[0][1] * d[1][0] * d[2][2] -
					d[0][0] * d[1][2] * d[2][1];

				if (dot < ZERO<T>) {
					for (auto i = 0; i < 3; ++i) {
						for (auto j = 0; j < 3; ++j) d[i][j] = -d[i][j];
					}
				}

				if constexpr (!std::same_as<RT, nullptr_t>) copy<Hint::NONE, 0, 0, 0, 0, 3, 3, true>(d, dstRot);

				if constexpr (!std::same_as<ST, nullptr_t>) {
					constexpr auto n = std::extent_v<std::remove_cvref_t<ST>, 0>;
					if constexpr (n > 0) xyz[0] = d[0][0] * m[0][0] + d[1][0] * m[1][0] + d[2][0] * m[2][0];
					if constexpr (n > 1) xyz[1] = d[0][1] * m[0][1] + d[1][1] * m[1][1] + d[2][1] * m[2][1];
					if constexpr (n > 2) xyz[2] = d[0][2] * m[0][2] + d[1][2] * m[1][2] + d[2][2] * m[2][2];

					if constexpr (n > 0) dstScale[0] = xyz[0];
					if constexpr (n > 1) dstScale[1] = xyz[1];
					if constexpr (n > 2) dstScale[2] = xyz[2];
				}

				return true;
			}
		}

		template<typename T>
		inline static constexpr FloatingPointType<T> SRK_CALL deg(const T& rad) {
			return rad * DEG<T>;
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

		template<typename In1, typename In2>
		inline static bool SRK_CALL equal(const In1& v1, const In2& v2) {
			return v1 == v2;
		}
		template<size_t N, typename In1, typename In2>
		inline static bool SRK_CALL equal(const In1(&v)[N], const In2& value) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v[i] != value) return false;
			}
			return true;
		}
		template<size_t N, typename In1, typename In2>
		inline static bool SRK_CALL equal(const In1(&v1)[N], const In2(&v2)[N]) {
			for (decltype(N) i = 0; i < N; ++i) {
				if (v1[i] != v2[i]) return false;
			}
			return true;
		}
		template<typename In1, typename In2, typename In3>
		inline static bool SRK_CALL equal(const In1& v1, const In2& v2, const In3& tolerance) {
			return (v1 < v2 ? v2 - v1 : v1 - v2) <= tolerance;
		}
		template<size_t N, typename In1, typename In2, typename In3>
		inline static bool SRK_CALL equal(const In1(&v)[N], const In2& value, const In3& tolerance) {
			for (decltype(N) i = 0; i < N; ++i) {
				if ((v[i] < value ? value - v[i] : v[i] - value) > tolerance) return false;
			}
			return true;
		}
		template<size_t N, typename In1, typename In2, typename In3>
		inline static bool SRK_CALL equal(const In1(&v1)[N], const In2(&v2)[N], const In3& tolerance) {
			for (decltype(N) i = 0; i < N; ++i) {
				if ((v1[i] < v2[i] ? v2[i] - v1[i] : v1[i] - v2[i]) > tolerance) return false;
			}
			return true;
		}

	private:
		template<size_t Cur, size_t RangeBegin, size_t Range, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL _identityInside(DstT(&dst)[DstTotalCs][DstTotalRs]) {
			if constexpr (Cur < Range) {
				if constexpr (Cur + RangeBegin < DstTotalRs * DstTotalCs) {
					constexpr auto c = (Cur + RangeBegin) / DstTotalRs;
					constexpr auto r = Cur + RangeBegin - c * DstTotalRs;
					if constexpr (c == r) {
						dst[c][r] = ONE<DstT>;
					} else {
						dst[c][r] = ZERO<DstT>;
					}
				}

				if constexpr (Cur + 1 < Range) {
					_identityInside<Cur + 1, RangeBegin, Range, DstTotalCs, DstTotalRs, DstT>(dst);
				}
			}
		}

		template<size_t Cur, size_t RangeBegin, size_t Range, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL _identityOutside(DstT(&dst)[DstTotalCs][DstTotalRs]) {
			if constexpr (Cur < DstTotalCs * DstTotalRs) {
				if constexpr (Cur < RangeBegin || Cur >= RangeBegin + Range) {
					constexpr auto c = (Cur + RangeBegin) / DstTotalRs;
					constexpr auto r = Cur + RangeBegin - c * DstTotalRs;
					if constexpr (c == r) {
						dst[c][r] = ONE<DstT>;
					} else {
						dst[c][r] = ZERO<DstT>;
					}
				}

				if constexpr (Cur + 1 < DstTotalCs * DstTotalRs) {
					_identityOutside<Cur + 1, RangeBegin, Range, DstTotalCs, DstTotalRs, DstT>(dst);
				}
			}
		}

	public:
		template<size_t RangeBegin, size_t Range, bool Inside, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL identity(DstT(&dst)[DstTotalCs][DstTotalRs]) {
			if constexpr (Inside) {
				_identityInside<0, RangeBegin, Range, DstTotalCs, DstTotalRs, DstT>(dst);
			} else {
				_identityOutside<0, RangeBegin, Range, DstTotalCs, DstTotalRs, DstT>(dst);
			}
		}

		template<size_t RangeBeginC, size_t RangeBeginR, size_t Range, bool Inside, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL identity(DstT(&dst)[DstTotalCs][DstTotalRs]) {
			identity<RangeBeginC * DstTotalRs + RangeBeginC, Range, Inside>(dst);
		}

	private:
		template<size_t CurC, size_t CurR, size_t RangeBeginC, size_t RangeBeginR, size_t RangeCs, size_t RangeRs, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL _identityInside(DstT(&dst)[DstTotalCs][DstTotalRs]) {
			if constexpr (CurR < RangeRs) {
				if constexpr (CurR + RangeBeginR < DstTotalRs && CurC + RangeBeginC < DstTotalCs) {
					if constexpr (CurC + RangeBeginC == CurR + RangeBeginR) {
						dst[CurC + RangeBeginC][CurR + RangeBeginR] = ONE<DstT>;
					} else {
						dst[CurC + RangeBeginC][CurR + RangeBeginR] = ZERO<DstT>;
					}
				}

				if constexpr (CurR + 1 == RangeRs) {
					if constexpr (CurC + 1 < RangeCs) {
						_identityInside<CurC + 1, 0, RangeBeginC, RangeBeginR, RangeCs, RangeRs, DstTotalCs, DstTotalRs, DstT>(dst);
					}
				} else {
					_identityInside<CurC, CurR + 1, RangeBeginC, RangeBeginR, RangeCs, RangeRs, DstTotalCs, DstTotalRs, DstT>(dst);
				}
			}
		}

		template<size_t CurC, size_t CurR, size_t RangeBeginC, size_t RangeBeginR, size_t RangeCs, size_t RangeRs, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL _identityOutside(DstT(&dst)[DstTotalCs][DstTotalRs]) {
			if constexpr (CurR < DstTotalRs) {
				if constexpr (CurR < RangeBeginR || CurR >= RangeBeginR + RangeRs || CurC < RangeBeginC || CurC >= RangeBeginC + RangeCs) {
					if constexpr (CurR + RangeBeginR < DstTotalRs && CurC + RangeBeginC < DstTotalCs) {
						if constexpr (CurC + RangeBeginC == CurR + RangeBeginR) {
							dst[CurC + RangeBeginC][CurR + RangeBeginR] = ONE<DstT>;
						} else {
							dst[CurC + RangeBeginC][CurR + RangeBeginR] = ZERO<DstT>;
						}
					}
				}

				if constexpr (CurR + 1 == DstTotalRs) {
					if constexpr (CurC + 1 < DstTotalCs) {
						_identityOutside<CurC + 1, 0, RangeBeginC, RangeBeginR, RangeCs, RangeRs, DstTotalCs, DstTotalRs, DstT>(dst);
					}
				} else {
					_identityOutside<CurC, CurR + 1, RangeBeginC, RangeBeginR, RangeCs, RangeRs, DstTotalCs, DstTotalRs, DstT>(dst);
				}
			}
		}

	public:
		template<size_t RangeBeginC, size_t RangeBeginR, size_t RangeCs, size_t RangeRs, bool Inside, size_t DstTotalCs, size_t DstTotalRs, std::floating_point DstT>
		inline static void SRK_CALL identity(DstT(&dst)[DstTotalCs][DstTotalRs]) {
			if constexpr (Inside) {
				_identityInside<0, 0, RangeBeginC, RangeBeginR, RangeCs, RangeRs, DstTotalCs, DstTotalRs, DstT>(dst);
			} else {
				_identityOutside<0, 0, RangeBeginC, RangeBeginR, RangeCs, RangeRs, DstTotalCs, DstTotalRs, DstT>(dst);
			}
		}

	private:
		template<typename T, typename Tmp, typename Det, size_t C, size_t R, typename Out>
		static void SRK_CALL _invert(const T(&m)[3][4], Out(&dst)[C][R], Tmp(&tmp)[3], Det det) {
			det = ONE<Det> / det;

			if constexpr (R > 0) {
				if constexpr (C > 0) dst[0][0] = tmp[0] * det;
				if constexpr (C > 1) dst[1][0] = tmp[1] * det;
				if constexpr (C > 2) dst[2][0] = tmp[2] * det;
			}

			if constexpr (R > 1) {
				if constexpr (C > 0) dst[0][1] = (m[2][1] * m[0][2] - m[2][2] * m[0][1]) * det;
				if constexpr (C > 1) dst[1][1] = (m[2][2] * m[0][0] - m[2][0] * m[0][2]) * det;
				if constexpr (C > 2) dst[2][1] = (m[2][0] * m[0][1] - m[2][1] * m[0][0]) * det;
			}

			tmp[0] = m[0][2] * m[1][3];
			tmp[1] = m[0][3] * m[1][2];
			tmp[2] = m[0][1] * m[1][3];
			auto tmp3 = m[0][3] * m[1][1];
			auto tmp4 = m[0][1] * m[1][2];
			auto tmp5 = m[0][2] * m[1][1];
			auto tmp6 = m[0][0] * m[1][3];
			auto tmp7 = m[0][3] * m[1][0];
			auto tmp8 = m[0][0] * m[1][2];
			auto tmp9 = m[0][2] * m[1][0];
			auto tmp10 = m[0][0] * m[1][1];
			auto tmp11 = m[0][1] * m[1][0];

			if constexpr (R > 2) {
				if constexpr (C > 0) dst[0][2] = (tmp4 - tmp5) * det;
				if constexpr (C > 1) dst[1][2] = (tmp9 - tmp8) * det;
				if constexpr (C > 2) dst[2][2] = (tmp10 - tmp11) * det;
			}

			if constexpr (R > 3) {
				if constexpr (C > 0) dst[0][3] = (tmp[2] * m[2][2] + tmp5 * m[2][3] + tmp[1] * m[2][1] - tmp4 * m[2][3] - tmp[0] * m[2][1] - tmp3 * m[2][2]) * det;
				if constexpr (C > 1) dst[1][3] = (tmp8 * m[2][3] + tmp[0] * m[2][0] + tmp7 * m[2][2] - tmp6 * m[2][2] - tmp9 * m[2][3] - tmp[1] * m[2][0]) * det;
				if constexpr (C > 2) dst[2][3] = (tmp6 * m[2][1] + tmp11 * m[2][3] + tmp3 * m[2][0] - tmp10 * m[2][3] - tmp[2] * m[2][0] - tmp7 * m[2][1]) * det;
			}
		}

		template<Hint Hints, std::floating_point T, size_t C, size_t R, std::floating_point Out>
		static bool SRK_CALL _invert(const T(&m)[3][4], Out(&dst)[C][R]) {
			using namespace srk::enum_operators;

			T tmp[3];
			tmp[0] = m[2][2] * m[1][1] - m[2][1] * m[1][2];
			tmp[1] = m[2][0] * m[1][2] - m[2][2] * m[1][0];
			tmp[2] = m[2][1] * m[1][0] - m[2][0] * m[1][1];

			if (Out det = dot((const T(&)[3])m[0], tmp); std::abs(det) > TOLERANCE<Out>) {
				if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					Out d[C][R];
					_invert(m, d, tmp, det);
					memcpy(dst, d, sizeof(dst));
				} else {
					_invert(m, dst, tmp, det);
				}

				return true;
			} else {
				return false;
			}
		}

		template<typename T, typename Tmp, typename Det, size_t C, size_t R, typename Out>
		static void SRK_CALL _invert(const T(&m)[4][4], Out(&dst)[C][R], Tmp(&tmp)[16], Det det) {
			det = ONE<Det> / det;

			if constexpr (R > 0) {
				if constexpr (C > 0) dst[0][0] = tmp[12] * det;
				if constexpr (C > 1) dst[1][0] = tmp[13] * det;
				if constexpr (C > 2) dst[2][0] = tmp[14] * det;
				if constexpr (C > 3) dst[3][0] = tmp[15] * det;
			}
			
			if constexpr (R > 1) {
				if constexpr (C > 0) dst[0][1] = (tmp[1] * m[0][1] + tmp[2] * m[0][2] + tmp[5] * m[0][3] - tmp[0] * m[0][1] - tmp[3] * m[0][2] - tmp[4] * m[0][3]) * det;
				if constexpr (C > 1) dst[1][1] = (tmp[0] * m[0][0] + tmp[7] * m[0][2] + tmp[8] * m[0][3] - tmp[1] * m[0][0] - tmp[6] * m[0][2] - tmp[9] * m[0][3]) * det;
				if constexpr (C > 2) dst[2][1] = (tmp[3] * m[0][0] + tmp[6] * m[0][1] + tmp[11] * m[0][3] - tmp[2] * m[0][0] - tmp[7] * m[0][1] - tmp[10] * m[0][3]) * det;
				if constexpr (C > 3) dst[3][1] = (tmp[4] * m[0][0] + tmp[9] * m[0][1] + tmp[10] * m[0][2] - tmp[5] * m[0][0] - tmp[8] * m[0][1] - tmp[11] * m[0][2]) * det;
			}

			tmp[0] = m[0][2] * m[1][3];
			tmp[1] = m[0][3] * m[1][2];
			tmp[2] = m[0][1] * m[1][3];
			tmp[3] = m[0][3] * m[1][1];
			tmp[4] = m[0][1] * m[1][2];
			tmp[5] = m[0][2] * m[1][1];
			tmp[6] = m[0][0] * m[1][3];
			tmp[7] = m[0][3] * m[1][0];
			tmp[8] = m[0][0] * m[1][2];
			tmp[9] = m[0][2] * m[1][0];
			tmp[10] = m[0][0] * m[1][1];
			tmp[11] = m[0][1] * m[1][0];

			if constexpr (R > 2) {
				if constexpr (C > 0) dst[0][2] = (tmp[0] * m[3][1] + tmp[3] * m[3][2] + tmp[4] * m[3][3] - tmp[1] * m[3][1] - tmp[2] * m[3][2] - tmp[5] * m[3][3]) * det;
				if constexpr (C > 1) dst[1][2] = (tmp[1] * m[3][0] + tmp[6] * m[3][2] + tmp[9] * m[3][3] - tmp[0] * m[3][0] - tmp[7] * m[3][2] - tmp[8] * m[3][3]) * det;
				if constexpr (C > 2) dst[2][2] = (tmp[2] * m[3][0] + tmp[7] * m[3][1] + tmp[10] * m[3][3] - tmp[3] * m[3][0] - tmp[6] * m[3][1] - tmp[11] * m[3][3]) * det;
				if constexpr (C > 3) dst[3][2] = (tmp[5] * m[3][0] + tmp[8] * m[3][1] + tmp[11] * m[3][2] - tmp[4] * m[3][0] - tmp[9] * m[3][1] - tmp[10] * m[3][2]) * det;
			}

			if constexpr (R > 3) {
				if constexpr (C > 0) dst[0][3] = (tmp[2] * m[2][2] + tmp[5] * m[2][3] + tmp[1] * m[2][1] - tmp[4] * m[2][3] - tmp[0] * m[2][1] - tmp[3] * m[2][2]) * det;
				if constexpr (C > 1) dst[1][3] = (tmp[8] * m[2][3] + tmp[0] * m[2][0] + tmp[7] * m[2][2] - tmp[6] * m[2][2] - tmp[9] * m[2][3] - tmp[1] * m[2][0]) * det;
				if constexpr (C > 2) dst[2][3] = (tmp[6] * m[2][1] + tmp[11] * m[2][3] + tmp[3] * m[2][0] - tmp[10] * m[2][3] - tmp[2] * m[2][0] - tmp[7] * m[2][1]) * det;
				if constexpr (C > 3) dst[3][3] = (tmp[10] * m[2][2] + tmp[4] * m[2][0] + tmp[9] * m[2][1] - tmp[8] * m[2][1] - tmp[11] * m[2][2] - tmp[5] * m[2][0]) * det;
			}
		}

		template<Hint Hints, std::floating_point T, size_t C, size_t R, std::floating_point Out>
		static bool SRK_CALL _invert(const T(&m)[4][4], Out(&dst)[C][R]) {
			using namespace srk::enum_operators;

			T tmp[16];
			tmp[0] = m[2][2] * m[3][3];
			tmp[1] = m[2][3] * m[3][2];
			tmp[2] = m[2][1] * m[3][3];
			tmp[3] = m[2][3] * m[3][1];
			tmp[4] = m[2][1] * m[3][2];
			tmp[5] = m[2][2] * m[3][1];
			tmp[6] = m[2][0] * m[3][3];
			tmp[7] = m[2][3] * m[3][0];
			tmp[8] = m[2][0] * m[3][2];
			tmp[9] = m[2][2] * m[3][0];
			tmp[10] = m[2][0] * m[3][1];
			tmp[11] = m[2][1] * m[3][0];

			tmp[12] = tmp[0] * m[1][1] + tmp[3] * m[1][2] + tmp[4] * m[1][3] - tmp[1] * m[1][1] - tmp[2] * m[1][2] - tmp[5] * m[1][3];//00
			tmp[13] = tmp[1] * m[1][0] + tmp[6] * m[1][2] + tmp[9] * m[1][3] - tmp[0] * m[1][0] - tmp[7] * m[1][2] - tmp[8] * m[1][3];//10
			tmp[14] = tmp[2] * m[1][0] + tmp[7] * m[1][1] + tmp[10] * m[1][3] - tmp[3] * m[1][0] - tmp[6] * m[1][1] - tmp[11] * m[1][3];//20
			tmp[15] = tmp[5] * m[1][0] + tmp[8] * m[1][1] + tmp[11] * m[1][2] - tmp[4] * m[1][0] - tmp[9] * m[1][1] - tmp[10] * m[1][2];//30

			auto tmp4 = tmp + 12;
			if (Out det = dot((const T(&)[4])m[0], (const T(&)[4])tmp4); std::abs(det) > TOLERANCE<Out>) {
				det = ONE<Out> / det;

				if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
					Out d[C][R];
					_invert(m, dst, tmp, det);
					memcpy(dst, d, sizeof(dst));
				} else {
					_invert(m, dst, tmp, det);
				}

				return true;
			} else {
				return false;
			}
		}

	public:
		template<Hint Hints = Hint::NONE, size_t SC, size_t SR, std::floating_point ST, size_t DC, size_t DR, std::floating_point DT>
		inline static bool SRK_CALL invert(const ST(&m)[SC][SR], DT(&dst)[DC][DR]) {
			if constexpr (SC == 3) {
				if constexpr (SR == 4) {
					return _invert<Hints>(m, dst);
				} else {
					return false;
				}
			} else if constexpr (SC == 4) {
				if constexpr (SR == 4) {
					return _invert<Hints>(m, dst);
				} else {
					return false;
				}
			} else {
				return false;
			}
		}

		template<size_t N, typename In1, typename In2, typename In3, typename Out>
		inline static void SRK_CALL lerp(const In1(&from)[N], const In2(&to)[N], const In3 t, Out(&dst)[N]) {
			Out tmp[N];
			for (decltype(N) i = 0; i < N; ++i) tmp[i] = from[i] + (to[i] - from[i]) * t;
			for (decltype(N) i = 0; i < N; ++i) dst[i] = tmp[i];
		}

		template<Arithmetic T>
		inline static T SRK_CALL max(const T& a, const T& b) {
			return std::max(a, b);
		}

		template<size_t N, typename In, typename Out = In>
		inline static void SRK_CALL max(const In(&a)[N], const In(&b)[N], Out(&dst)[N]) {
			for (decltype(N) i = 0; i < N; ++i) dst[i] = max(a[i], b[i]);
		}

		template<std::floating_point T>
		static void SRK_CALL appendQuat(const T(&lhs)[4], const T(&rhs)[4], T(&dst)[4]) {
			auto w = lhs[3] * rhs[3] - lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2];
			auto x = lhs[0] * rhs[3] + lhs[3] * rhs[0] + lhs[2] * rhs[1] - lhs[1] * rhs[2];
			auto y = lhs[1] * rhs[3] + lhs[3] * rhs[1] + lhs[0] * rhs[2] - lhs[2] * rhs[0];
			auto z = lhs[2] * rhs[3] + lhs[3] * rhs[2] + lhs[1] * rhs[0] - lhs[0] * rhs[1];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = w;
		}

		template<std::floating_point T>
		static void SRK_CALL rotateQuat(const T(&q)[4], const T(&p)[3], float32_t(&dst)[3]) {
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

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		static void SRK_CALL mul(const L(&lhs)[3][4], const R(&rhs)[3][4], Out(&dst)[3][4]) {
			Out m[3][4];

			for (uint8_t c = 0; c < 3; ++c) {
				auto& mc = m[c];
				auto& rc = rhs[c];
				mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2];
				mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2];
				mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2];
				mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + rc[3];
			}

			memcpy(dst, m, sizeof(m));
		}

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		static void SRK_CALL mul(const L(&lhs)[3][4], const R(&rhs)[3][4], Out(&dst)[4][4]) {
			mul(lhs, rhs, (Out(&)[3][4])dst);

			dst[3][0] = ZERO<Out>;
			dst[3][1] = ZERO<Out>;
			dst[3][2] = ZERO<Out>;
			dst[3][3] = ONE<Out>;
		}

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		inline static void SRK_CALL mul(const L(&lhs)[3][4], const R(&rhs)[4][4], Out(&dst)[3][4]) {
			mul(lhs, (R(&)[3][4])rhs, (Out(&)[3][4])dst);
		}

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		static void SRK_CALL mul(const L(&lhs)[3][4], const R(&rhs)[4][4], Out(&dst)[4][4]) {
			Out m[4][4];

			for (uint8_t c = 0; c < 4; ++c) {
				auto& mc = m[c];
				auto& rc = rhs[c];
				mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2];
				mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2];
				mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2];
				mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + rc[3];
			}

			memcpy(dst, m, sizeof(m));
		}

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		static void SRK_CALL mul(const L(&lhs)[4][4], const R(&rhs)[3][4], Out(&dst)[3][4]) {
			Out m[3][4];

			for (uint8_t c = 0; c < 3; ++c) {
				auto& mc = m[c];
				auto& rc = rhs[c];
				mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2] + lhs[3][0] * rc[3];
				mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2] + lhs[3][1] * rc[3];
				mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2] + lhs[3][2] * rc[3];
				mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + lhs[3][3] * rc[3];
			}

			memcpy(dst, m, sizeof(m));
		}

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		static void SRK_CALL mul(const L(&lhs)[4][4], const R(&rhs)[3][4], Out(&dst)[4][4]) {
			mul(lhs, rhs, (Out(&)[3][4])dst);

			dst[3][0] = lhs[3][0];
			dst[3][1] = lhs[3][1];
			dst[3][2] = lhs[3][2];
			dst[3][3] = lhs[3][3];
		}

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		inline static void SRK_CALL mul(const L(&lhs)[4][4], const R(&rhs)[4][4], Out(&dst)[3][4]) {
			mul(lhs, (R(&)[3][4])rhs, dst);
		}

		template<std::floating_point L, std::floating_point R, std::floating_point Out>
		static void SRK_CALL mul(const L(&lhs)[4][4], const R(&rhs)[4][4], Out(&dst)[4][4]) {
			Out m[4][4];

			for (uint8_t c = 0; c < 4; ++c) {
				auto& mc = m[c];
				auto& rc = rhs[c];
				mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2] + lhs[3][0] * rc[3];
				mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2] + lhs[3][1] * rc[3];
				mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2] + lhs[3][2] * rc[3];
				mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + lhs[3][3] * rc[3];
			}

			memcpy(dst, m, sizeof(dst));
		}

		inline static void SRK_CALL mul(const float32_t(&m)[3][4], const float32_t(&p)[3], float32_t(&dst)[3]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
		}

		inline static void SRK_CALL mul(const float32_t(&m)[4][4], const float32_t(&p)[4], float32_t(&dst)[4]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + p[3] * m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + p[3] * m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + p[3] * m[2][3];
			auto w = p[0] * m[3][0] + p[1] * m[3][1] + p[2] * m[3][2] + p[3] * m[3][3];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = w;
		}

		inline static void SRK_CALL mul(const float32_t(&m)[4][4], const float32_t(&p)[3], float32_t(&dst)[4]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];
			auto w = p[0] * m[3][0] + p[1] * m[3][1] + p[2] * m[3][2] + m[3][3];

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = w;
		}

		inline static void SRK_CALL mul(const float32_t(&m)[4][4], const float32_t(&p)[3], float32_t(&dst)[3]) {
			auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
			auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
			auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];
			auto w = p[0] * m[3][0] + p[1] * m[3][1] + p[2] * m[3][2] + m[3][3];

			dst[0] = x / w;
			dst[1] = y / w;
			dst[2] = z / w;
		}

		template<size_t N, typename In, typename Out = In>
		inline static constexpr Out SRK_CALL mul(const In(&v)[N]) {
			if constexpr (N == 0) {
				return Out(0);
			} else {
				Out m = v[0];
				for (decltype(N) i = 1; i < N; ++i) m *= v[i];
				return m;
			}
		}

		template<size_t N, typename In, typename Out>
		static void SRK_CALL normalize(const In(&v)[N], Out(&dst)[N]) {
			if constexpr (sizeof(In) >= sizeof(Out)) {
				if (auto n = dot<N, In, In, Out>(v, v); !equal(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
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
				if (auto n = dot<N, In, In, Out>(v, v); !equal(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
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
			if (auto n = dot(val, val); !equal(n, ONE<decltype(n)>, TOLERANCE<decltype(n)>)) {
				n = std::sqrt(n);
				if (n > TOLERANCE<decltype(n)>) {
					n = ONE<decltype(n)> / n;

					for (decltype(N) i = 0; i < N; ++i) val[i] *= n;
				}
			}
		}

		inline static constexpr uint32_t SRK_CALL potLog2(uint32_t pow) {
			return ((pow >> 23) & 0xFF) - 127;
		}

		template<typename T>
		inline static constexpr FloatingPointType<T> SRK_CALL rad(const T& deg) {
			return deg * RAD<T>;
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

		template<std::floating_point From, std::floating_point To, std::floating_point T, std::floating_point Out>
		static void SRK_CALL slerp(const From(&from)[4], const To(&to)[4], T t, Out(&dst)[4]) {
			auto x = to[0], y = to[1], z = to[2], w = to[3];
			auto cos = from[0] * x + from[1] * y + from[2] * z + from[3] * w;
			if (cos < ZERO<Out>) {//shortest path
				x = -x;
				y = -y;
				z = -z;
				w = -w;
				cos = -cos;
			}

			Out k0, k1;
			if (cos > (decltype(cos)).9999f) {
				k0 = ONE<Out> - t;
				k1 = t;
			} else {
				auto a = std::acos(cos);
				auto s = std::sin(a);
				auto ta = t * a;
				k0 = std::sin(a - ta) / s;
				k1 = std::sin(ta) / s;
			}

			x = from[0] * k0 + x * k1;
			y = from[1] * k0 + y * k1;
			z = from[2] * k0 + z * k1;

			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst[3] = from[3] * k0 + w * k1;
		}

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

	private:
		template<size_t SC, size_t SR, std::floating_point ST, size_t DC, size_t DR, std::floating_point DT>
		static void SRK_CALL _transpose(const ST(&src)[SC][SR], DT(&dst)[DC][DR]) {
			constexpr auto nc = std::min(SR, DR);
			constexpr auto nr = std::min(SC, DC);
			for (std::remove_cvref_t<decltype(nc)> c = 0; c < nc; ++c) {
				for (std::remove_cvref_t<decltype(nr)> r = 0; r < nr; ++r) dst[c][r] = src[r][c];
			}
		}

	public:
		template<Hint Hints = Hint::NONE, size_t SC, size_t SR, std::floating_point ST, size_t DC, size_t DR, std::floating_point DT>
		static void SRK_CALL transpose(const ST(&src)[SC][SR], DT(&dst)[DC][DR]) {
			using namespace srk::enum_operators;

			if constexpr ((Hints & Hint::MEM_OVERLAP) == Hint::MEM_OVERLAP) {
				constexpr auto nc = std::min(SR, DR);
				constexpr auto nr = std::min(SC, DC);
				DT d[nc][nr];
				_transpose(src, d);
				copy<Hints, 0, 0, 0, 0, nc, nr, true, nc, nr, DT>(d, dst);
			} else {
				_transpose(src, dst);
			}
		}
	};
}