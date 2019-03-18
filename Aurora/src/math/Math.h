#pragma once

#include "base/Aurora.h"
#include <cmath>

namespace aurora {
	class AE_DLL Math {
	public:
		Math() = delete;
		Math(const Math&) = delete;
		Math(Math&&) = delete;

		static const f32 F32_TOLERANCE;
		static const f64 PI;
		static const f64 PI_2;
		static const f64 PI_4;
		static const f64 PI2;
		static const f64 F64_DEG;
		static const f32 F32_DEG;
		static const f64 F64_RAD;
		static const f32 F32_RAD;

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

		inline static constexpr f32 AE_CALL deg(f32 rad);
		inline static constexpr f32 AE_CALL rad(f32 deg);
	};
}

#include "Math.inl"