#pragma once

#include <initializer_list>
#include "base/Aurora.h"
//#include <xmmintrin.h>

AE_NS_BEGIN

class Matrix44;
class Quaternion;
class Vector3;

/**
 * column major matrix.
 *
 * m00 m10 m20 m30 xaxis
 * m01 m11 m21 m31 yaxis
 * m02 m12 m22 m32 zaxis
 * m03 m13 m23 m33
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

	void AE_CALL decomposition(Matrix34* dstRot, Vector3* dstScale = nullptr) const;

	void AE_CALL toQuaternion(Quaternion& dst) const;

	inline void AE_CALL append(const Matrix34& rhs);
	inline void AE_CALL append(const Matrix44& rhs);
	inline void AE_CALL append(const Matrix34& rhs, Matrix34& dst) const;
	inline void AE_CALL append(const Matrix34& rhs, Matrix44& dst) const;
	inline void AE_CALL append(const Matrix44& rhs, Matrix34& dst) const;
	inline void AE_CALL append(const Matrix44& rhs, Matrix44& dst) const;

	inline void AE_CALL appendTranslate(const Vector3& t);
	inline void AE_CALL prependTranslate(const Vector3& t);
	inline void AE_CALL setPosition(const Vector3& p);
	inline void AE_CALL setPosition(const f32* p);
	inline void AE_CALL setPosition(const Matrix34& m);
	inline void AE_CALL setPosition(const Matrix44& m);

	inline void AE_CALL prependScale(const Vector3& t);

	static void AE_CALL createLookAt(const Vector3& forward, const Vector3& upward, Matrix34& dst);
	static void AE_CALL createRotationAxis(const Vector3& axis, f32 radian, Matrix34& dst);

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

	static void AE_CALL createScale(const Vector3& scale, Matrix34& dst);
	static void AE_CALL createTranslation(const Vector3& trans, Matrix34& dst);
	static void AE_CALL createTRS(const Vector3* trans, const Quaternion* rot, const Vector3* scale, Matrix34& dst);

	union {
		//__m128 col[4];
		f32 m34[3][4];
	};
};

AE_NS_END

#include "Matrix34.inl"