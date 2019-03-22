#pragma once

#include "base/Aurora.h"
#include <cmath>

namespace aurora {
	class AE_DLL Math {
	public:
		Math() = delete;
		Math(const Math&) = delete;
		Math(Math&&) = delete;

		template<typename T>
		using FLOATING_POINT_TYPE = typename std::enable_if_t<std::is_floating_point_v<T>, T>;

		template<typename T> inline static const FLOATING_POINT_TYPE<T> TOLERANCE = T(2e-37);
		template<typename T> inline static const FLOATING_POINT_TYPE<T> PI = T(3.14159265358979323846);
		template<typename T> inline static const FLOATING_POINT_TYPE<T> PI_2 = PI<T> * T(.5);
		template<typename T> inline static const FLOATING_POINT_TYPE<T> PI_4 = PI<T> * T(.25);
		template<typename T> inline static const FLOATING_POINT_TYPE<T> PI2 = PI<T> * T(2.);
		template<typename T> inline static const FLOATING_POINT_TYPE<T> DEG = T(180.) / PI<T>;
		template<typename T> inline static const FLOATING_POINT_TYPE<T> RAD = PI<T> / T(180.);

		inline static void AE_CALL crossVec3(const f32* v1, const f32* v2, f32* dst);
		inline static void AE_CALL lerpVec3(const f32* from, const f32* to, f32 t, f32* dst);

		static void AE_CALL slerpQuat(const f32* from, const f32* to, f32 t, f32* dst);
		inline static void AE_CALL appendQuat(const f32* lhs, const f32* rhs, f32* dst);
		inline static void AE_CALL quatRotateVec3(const f32* q, const f32* p, f32* dst);

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
		inline static constexpr FLOATING_POINT_TYPE<T> AE_CALL deg(T rad) {
			return rad * DEG<T>;
		}

		template<typename T>
		inline static constexpr FLOATING_POINT_TYPE<T> AE_CALL rad(T deg) {
			return deg * RAD<T>;
		}
	};
}

#include "Math.inl"