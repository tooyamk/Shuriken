#pragma once

#include "base/LowLevel.h"
#include <initializer_list>

namespace aurora {
	class Matrix44;
	class Quaternion;

	/**
	 * column major matrix.
	 *
	 * m00 m10 m20 xaxis
	 * m01 m11 m21 yaxis
	 * m02 m12 m22 zaxis
	 * m03 m13 m23
	 * tx  ty  tz
	 */
	class AE_DLL Matrix34 {
	public:
		Matrix34();
		Matrix34(
			f32 m00, f32 m01 = 0.f, f32 m02 = 0.f, f32 m03 = 0.f,
			f32 m10 = 0.f, f32 m11 = 1.f, f32 m12 = 0.f, f32 m13 = 0.f,
			f32 m20 = 0.f, f32 m21 = 0.f, f32 m22 = 1.f, f32 m23 = 0.f);
		Matrix34(const Matrix34& m);
		Matrix34(const Matrix44& m);
		Matrix34(const std::initializer_list<f32>& m);
		Matrix34(const f32(&m)[3][4]);
		Matrix34(const f32(&m)[4][4]);
		~Matrix34();

		void AE_CALL set33(const Matrix34& m);
		void AE_CALL set33(const Matrix44& m);
		void AE_CALL set33(const f32(&m)[3][3]);
		void AE_CALL set33(
			f32 m00 = 1.f, f32 m01 = 0.f, f32 m02 = 0.f,
			f32 m10 = 0.f, f32 m11 = 1.f, f32 m12 = 0.f,
			f32 m20 = 0.f, f32 m21 = 0.f, f32 m22 = 1.f);
		void AE_CALL set34(const Matrix34& m);
		void AE_CALL set34(const Matrix44& m);
		void AE_CALL set34(
			f32 m00 = 1.f, f32 m01 = 0.f, f32 m02 = 0.f, f32 m03 = 0.f,
			f32 m10 = 0.f, f32 m11 = 1.f, f32 m12 = 0.f, f32 m13 = 0.f,
			f32 m20 = 0.f, f32 m21 = 0.f, f32 m22 = 1.f, f32 m23 = 0.f);

		inline void AE_CALL transpose(Matrix44& dst) const;

		inline bool AE_CALL invert();
		inline bool AE_CALL invert(Matrix34& dst) const;
		inline bool AE_CALL invert(Matrix44& dst) const;

		void AE_CALL decomposition(Matrix34* dstRot, f32(dstScale[3]) = nullptr) const;

		void AE_CALL toQuaternion(Quaternion& dst) const;

		inline void AE_CALL append(const Matrix34& rhs);
		inline void AE_CALL append(const Matrix44& rhs);
		inline void AE_CALL append(const Matrix34& rhs, Matrix34& dst) const;
		inline void AE_CALL append(const Matrix34& rhs, Matrix44& dst) const;
		inline void AE_CALL append(const Matrix44& rhs, Matrix34& dst) const;
		inline void AE_CALL append(const Matrix44& rhs, Matrix44& dst) const;

		inline void AE_CALL appendTranslate(const f32(&t)[3]);
		inline void AE_CALL prependTranslate(const f32(&t)[3]);
		inline void AE_CALL setPosition(const f32(&p)[3]);
		inline void AE_CALL setPosition(const Matrix34& m);
		inline void AE_CALL setPosition(const Matrix44& m);
		inline void AE_CALL setPosition(f32 x, f32 y, f32 z);

		inline void AE_CALL prependScale(const f32(&s)[3]);

		static void AE_CALL createLookAt(const f32(&forward)[3], const f32(&upward)[3], Matrix34& dst);
		static void AE_CALL createRotationAxis(const f32(&axis)[3], f32 radian, Matrix34& dst);

		/**
		 * direction(LH):(0, 1, 0) to (0, 0, 1)
		 */
		static void AE_CALL createRotationX(f32 radian, Matrix34& dst);

		/**
		 * direction(LH):(1, 0, 0) to (0, 0, -1)
		 */
		static void AE_CALL createRotationY(f32 radian, Matrix34& dst);

		/**
		 * direction(LH):(1, 0, 0) to (0, 1, 0)
		 */
		static void AE_CALL createRotationZ(f32 radian, Matrix34& dst);

		static void AE_CALL createScale(const f32(&scale)[3], Matrix34& dst);
		static void AE_CALL createTranslation(const f32(&trans)[3], Matrix34& dst);
		static void AE_CALL createTRS(const f32(&trans)[3], const Quaternion* rot, const f32(&scale)[3], Matrix34& dst);

		union {
			//__m128 col[4];
			f32 data[3][4];
		};
	};
}

#include "Matrix34.inl"