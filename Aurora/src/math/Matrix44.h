#pragma once

#include "base/LowLevel.h"
#include <initializer_list>

namespace aurora {
	class Matrix34;
	class Quaternion;

	/**
	 * column major matrix.
	 *
	 * m00 m10 m20 m30 xaxis
	 * m01 m11 m21 m31 yaxis
	 * m02 m12 m22 m32 zaxis
	 * m03 m13 m23 m33
	 * tx  ty  tz
	 */
	class AE_DLL Matrix44 {
	public:
		Matrix44();
		Matrix44(
			f32 m00, f32 m01 = 0.f, f32 m02 = 0.f, f32 m03 = 0.f,
			f32 m10 = 0.f, f32 m11 = 1.f, f32 m12 = 0.f, f32 m13 = 0.f,
			f32 m20 = 0.f, f32 m21 = 0.f, f32 m22 = 1.f, f32 m23 = 0.f,
			f32 m30 = 0.f, f32 m31 = 0.f, f32 m32 = 0.f, f32 m33 = 1.f);
		Matrix44(const Matrix34& m);
		Matrix44(const Matrix44& m);
		Matrix44(const std::initializer_list<f32>& m);
		Matrix44(const f32(&m)[3][4]);
		Matrix44(const f32(&m)[4][4]);
		~Matrix44();

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
		void AE_CALL set44(const Matrix34& m);
		void AE_CALL set44(const Matrix44& m);
		void AE_CALL set44(
			f32 m00 = 1.f, f32 m01 = 0.f, f32 m02 = 0.f, f32 m03 = 0.f,
			f32 m10 = 0.f, f32 m11 = 1.f, f32 m12 = 0.f, f32 m13 = 0.f,
			f32 m20 = 0.f, f32 m21 = 0.f, f32 m22 = 1.f, f32 m23 = 0.f,
			f32 m30 = 0.f, f32 m31 = 0.f, f32 m32 = 0.f, f32 m33 = 1.f);

		inline void AE_CALL transpose();
		inline void AE_CALL transpose(Matrix44& dst) const;

		inline bool AE_CALL invert();
		inline bool AE_CALL invert(Matrix44& dst) const;

		inline void AE_CALL append(const Matrix34& rhs);
		inline void AE_CALL append(const Matrix44& rhs);
		inline void AE_CALL append(const Matrix34& rhs, Matrix34& dst) const;
		inline void AE_CALL append(const Matrix34& rhs, Matrix44& dst) const;
		inline void AE_CALL append(const Matrix44& rhs, Matrix34& dst) const;
		inline void AE_CALL append(const Matrix44& rhs, Matrix44& dst) const;

		static void AE_CALL createOrthoLH(f32 width, f32 height, f32 zNear, f32 zFar, Matrix44& dst);
		static void AE_CALL createOrthoOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar, Matrix44& dst);
		static void AE_CALL createPerspectiveFovLH(f32 fieldOfViewY, f32 aspectRatio, f32 zNear, f32 zFar, Matrix44& dst);
		static void AE_CALL createPerspectiveLH(f32 width, f32 height, f32 zNear, f32 zFar, Matrix44& dst);
		static void AE_CALL createPerspectiveOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar, Matrix44& dst);
		//static void AE_CALL createLookAt(const Vector3& forward, const Vector3& upward, Matrix44& dst);
		//static void AE_CALL createRotationAxis(const Vector3& axis, f32 radian, Matrix44& dst);

		/**
		 * direction(LH):(0, 1, 0) to (0, 0, 1)
		 */
		 //static void AE_CALL createRotationX(f32 radian, Matrix44& dst);

		 /**
		  * direction(LH):(1, 0, 0) to (0, 0, -1)
		  */
		  //static void AE_CALL createRotationY(f32 radian, Matrix44& dst);

		  /**
		   * direction(LH):(1, 0, 0) to (0, 1, 0)
		   */
		   //static void AE_CALL createRotationZ(f32 radian, Matrix44& dst);

		   //static void AE_CALL createScale(const Vector3& scale, Matrix44& dst);
		   //static void AE_CALL createTranslation(const Vector3& trans, Matrix44& dst);
		   //static void AE_CALL createTRS(const Vector3* trans, const Quaternion* rot, const Vector3* scale, Matrix44& dst);

		union {
			//__m128 col[4];
			f32 data[4][4];
		};
	};
}

#include "Matrix44.inl"