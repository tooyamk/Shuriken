#pragma once

#include "aurora/math/Math.h"
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
		using Data = f32[3][4];

		Matrix34();
		Matrix34(const NoInit&);
		Matrix34(
			f32 m00,       f32 m01 = 0.f, f32 m02 = 0.f, f32 m03 = 0.f,
			f32 m10 = 0.f, f32 m11 = 1.f, f32 m12 = 0.f, f32 m13 = 0.f,
			f32 m20 = 0.f, f32 m21 = 0.f, f32 m22 = 1.f, f32 m23 = 0.f);
		Matrix34(const Matrix34& m);
		Matrix34(const Matrix44& m);
		Matrix34(const std::initializer_list<f32>& m);
		Matrix34(const f32(&m)[3][4]);
		Matrix34(const f32(&m)[4][4]);
		~Matrix34();

		static const Matrix34 IDENTITY;

		inline AE_CALL operator Data& ();
		inline AE_CALL operator const Data& () const;

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
			Data data;
		};
	};


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
		using Data34 = f32[3][4];
		using Data44 = f32[4][4];

		Matrix44();
		Matrix44(const NoInit&);
		Matrix44(
			f32 m00,       f32 m01 = 0.f, f32 m02 = 0.f, f32 m03 = 0.f,
			f32 m10 = 0.f, f32 m11 = 1.f, f32 m12 = 0.f, f32 m13 = 0.f,
			f32 m20 = 0.f, f32 m21 = 0.f, f32 m22 = 1.f, f32 m23 = 0.f,
			f32 m30 = 0.f, f32 m31 = 0.f, f32 m32 = 0.f, f32 m33 = 1.f);
		Matrix44(const Matrix34& m);
		Matrix44(const Matrix44& m);
		Matrix44(const std::initializer_list<f32>& m);
		Matrix44(const f32(&m)[3][4]);
		Matrix44(const f32(&m)[4][4]);
		~Matrix44();

		static const Matrix44 IDENTITY;

		inline AE_CALL operator Data44& ();
		inline AE_CALL operator const Data44& () const;

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

		inline static void AE_CALL createOrthoLH(f32 width, f32 height, f32 zNear, f32 zFar, Matrix44& dst) {
			dst.set44(
				2.f / width, 0.f, 0.f, 0.f,
				0.f, 2.f / height, 0.f, 0.f,
				0.f, 0.f, 1.f / (zFar - zNear), zNear / (zNear - zFar));
		}
		inline static Matrix44 AE_CALL createOrthoLH(f32 width, f32 height, f32 zNear, f32 zFar) {
			Matrix44 m(NO_INIT);
			createOrthoLH(width, height, zNear, zFar, m);
			return m;
		}
		inline static void AE_CALL createOrthoOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar, Matrix44& dst) {
			dst.set44(
				2.f / (right - 1.f), 0.f, 0.f, (1.f + right) / (1.f - right),
				0.f, 2.f / (top - bottom), 0.f, (top + bottom) / (bottom - top),
				0.f, 0.f, 1.f / (zFar - zNear), zNear / (zNear - zFar));
		}
		inline static Matrix44 AE_CALL createOrthoOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar) {
			Matrix44 m(NO_INIT);
			createOrthoOffCenterLH(left, right, bottom, top, zNear, zFar, m);
			return m;
		}
		inline static void AE_CALL createPerspectiveFovLH(f32 fieldOfViewY, f32 aspectRatio, f32 zNear, f32 zFar, Matrix44& dst) {
			f32 yScale = 1.f / std::tan(fieldOfViewY * .5f);
			dst.set44(
				yScale / aspectRatio, 0.f, 0.f, 0.f,
				0.f, yScale, 0.f, 0.f,
				0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
				0.f, 0.f, 1.f, 0.f);
		}
		inline static Matrix44 AE_CALL createPerspectiveFovLH(f32 fieldOfViewY, f32 aspectRatio, f32 zNear, f32 zFar) {
			Matrix44 m(NO_INIT);
			createPerspectiveFovLH(fieldOfViewY, aspectRatio, zNear, zFar, m);
			return m;
		}
		inline static void AE_CALL createPerspectiveLH(f32 width, f32 height, f32 zNear, f32 zFar, Matrix44& dst) {
			auto zNear2 = zNear * 2.f;
			dst.set44(
				zNear2 / width, 0.f, 0.f, 0.f,
				0.f, zNear2 / height, 0.f, 0.f,
				0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
				0.f, 0.f, 1.f, 0.f);
		}
		inline static Matrix44 AE_CALL createPerspectiveLH(f32 width, f32 height, f32 zNear, f32 zFar) {
			Matrix44 m(NO_INIT);
			createPerspectiveLH(width, height, zNear, zFar, m);
			return m;
		}
		inline static void AE_CALL createPerspectiveOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar, Matrix44& dst) {
			auto zNear2 = zNear * 2.f;
			dst.set44(
				zNear2 / (right - left), 0.f, (left + right) / (left - right), 0.f,
				0.f, zNear2 / (top - bottom), (top + bottom) / (bottom - top), 0.f,
				0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
				0.f, 0.f, 1.f, 0.f);
		}
		inline static Matrix44 AE_CALL createPerspectiveOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar) {
			Matrix44 m(NO_INIT);
			createPerspectiveOffCenterLH(left, right, bottom, top, zNear, zFar, m);
			return m;
		}
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
			Data44 data;
		};
	};


	inline AE_CALL Matrix34::operator Matrix34::Data& () {
		return data;
	}

	inline AE_CALL Matrix34::operator const Matrix34::Data& () const {
		return data;
	}

	inline void AE_CALL Matrix34::transpose(Matrix44& dst) const {
		Math::transposeMat(data, dst.data);
	}

	inline bool AE_CALL Matrix34::invert() {
		return Math::invertMat(data, data);
	}

	inline bool AE_CALL Matrix34::invert(Matrix34& dst) const {
		return Math::invertMat(data, dst.data);
	}

	inline bool AE_CALL Matrix34::invert(Matrix44& dst) const {
		return Math::invertMat(data, ((Matrix34&)dst).data);
	}

	inline void AE_CALL Matrix34::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void AE_CALL Matrix34::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void AE_CALL Matrix34::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix34::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix34::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix34::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix34::appendTranslate(const f32(&t)[3]) {
		data[0][3] += t[0];
		data[1][3] += t[1];
		data[2][3] += t[2];
	}

	inline void AE_CALL Matrix34::prependTranslate(const f32(&t)[3]) {
		auto x = t[0], y = t[1], z = t[2];
		data[0][3] += x * data[0][0] + y * data[0][1] + z * data[0][2];
		data[1][3] += x * data[1][0] + y * data[1][1] + z * data[1][2];
		data[2][3] += x * data[2][0] + y * data[2][1] + z * data[2][2];
	}

	inline void AE_CALL Matrix34::setPosition(const f32(&p)[3]) {
		setPosition(p[0], p[1], p[2]);
	}

	inline void AE_CALL Matrix34::setPosition(const Matrix34& m) {
		auto mm = m.data;
		setPosition(mm[0][3], mm[1][3], mm[2][3]);
	}

	inline void AE_CALL Matrix34::setPosition(const Matrix44& m) {
		auto mm = m.data;
		setPosition(mm[0][3], mm[1][3], mm[2][3]);
	}

	inline void AE_CALL Matrix34::setPosition(f32 x, f32 y, f32 z) {
		data[0][3] = x;
		data[1][3] = y;
		data[2][3] = z;
	}

	inline void AE_CALL Matrix34::prependScale(const f32(&s)[3]) {
		auto x = s[0], y = s[1], z = s[2];

		data[0][0] *= x;
		data[0][1] *= y;
		data[0][2] *= z;

		data[1][0] *= x;
		data[1][1] *= y;
		data[1][2] *= z;

		data[2][0] *= x;
		data[2][1] *= y;
		data[2][2] *= z;
	}

	inline AE_CALL Matrix44::operator Matrix44::Data44& () {
		return data;
	}

	inline AE_CALL Matrix44::operator const Matrix44::Data44& () const {
		return data;
	}

	inline void AE_CALL Matrix44::transpose() {
		Math::transposeMat(data, this->data);
	}

	inline void AE_CALL Matrix44::transpose(Matrix44& dst) const {
		Math::transposeMat(data, dst.data);
	}

	inline bool AE_CALL Matrix44::invert() {
		return Math::invertMat(data, data);
	}

	inline bool AE_CALL Matrix44::invert(Matrix44& dst) const {
		return Math::invertMat(data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void AE_CALL Matrix44::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void AE_CALL Matrix44::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}
}