#pragma once

#include "srk/math/Math.h"
#include <initializer_list>

namespace srk {
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
	class SRK_CORE_DLL Matrix34 {
	public:
		using Data = float32_t[3][4];

		Matrix34();
		Matrix34(const NoInit&);
		Matrix34(
			float32_t m00,       float32_t m01 = 0.f, float32_t m02 = 0.f, float32_t m03 = 0.f,
			float32_t m10 = 0.f, float32_t m11 = 1.f, float32_t m12 = 0.f, float32_t m13 = 0.f,
			float32_t m20 = 0.f, float32_t m21 = 0.f, float32_t m22 = 1.f, float32_t m23 = 0.f);
		Matrix34(const Matrix34& m);
		Matrix34(const Matrix44& m);
		Matrix34(const std::initializer_list<float32_t>& m);
		Matrix34(const float32_t(&m)[3][4]);
		Matrix34(const float32_t(&m)[4][4]);
		~Matrix34();

		static const Matrix34 IDENTITY;

		inline SRK_CALL operator Data& ();
		inline SRK_CALL operator const Data& () const;

		void SRK_CALL set33(const Matrix34& m);
		void SRK_CALL set33(const Matrix44& m);
		void SRK_CALL set33(const float32_t(&m)[3][3]);
		void SRK_CALL set33(
			float32_t m00 = 1.f, float32_t m01 = 0.f, float32_t m02 = 0.f,
			float32_t m10 = 0.f, float32_t m11 = 1.f, float32_t m12 = 0.f,
			float32_t m20 = 0.f, float32_t m21 = 0.f, float32_t m22 = 1.f);
		void SRK_CALL set34(const Matrix34& m);
		void SRK_CALL set34(const Matrix44& m);
		void SRK_CALL set34(
			float32_t m00 = 1.f, float32_t m01 = 0.f, float32_t m02 = 0.f, float32_t m03 = 0.f,
			float32_t m10 = 0.f, float32_t m11 = 1.f, float32_t m12 = 0.f, float32_t m13 = 0.f,
			float32_t m20 = 0.f, float32_t m21 = 0.f, float32_t m22 = 1.f, float32_t m23 = 0.f);

		inline void SRK_CALL transpose(Matrix44& dst) const;

		inline bool SRK_CALL invert();
		inline bool SRK_CALL invert(Matrix34& dst) const;
		inline bool SRK_CALL invert(Matrix44& dst) const;

		void SRK_CALL decomposition(Matrix34* dstRot, float32_t(dstScale[3]) = nullptr) const;

		void SRK_CALL toQuaternion(Quaternion& dst) const;

		inline void SRK_CALL append(const Matrix34& rhs);
		inline void SRK_CALL append(const Matrix44& rhs);
		inline void SRK_CALL append(const Matrix34& rhs, Matrix34& dst) const;
		inline void SRK_CALL append(const Matrix34& rhs, Matrix44& dst) const;
		inline void SRK_CALL append(const Matrix44& rhs, Matrix34& dst) const;
		inline void SRK_CALL append(const Matrix44& rhs, Matrix44& dst) const;

		inline void SRK_CALL appendTranslate(const float32_t(&t)[3]);
		inline void SRK_CALL prependTranslate(const float32_t(&t)[3]);
		inline void SRK_CALL setPosition(const float32_t(&p)[3]);
		inline void SRK_CALL setPosition(const Matrix34& m);
		inline void SRK_CALL setPosition(const Matrix44& m);
		inline void SRK_CALL setPosition(float32_t x, float32_t y, float32_t z);

		inline void SRK_CALL prependScale(const float32_t(&s)[3]);

		static void SRK_CALL createLookAt(const float32_t(&forward)[3], const float32_t(&upward)[3], Matrix34& dst);
		static void SRK_CALL createRotationAxis(const float32_t(&axis)[3], float32_t radian, Matrix34& dst);

		/**
		 * direction(LH):(0, 1, 0) to (0, 0, 1)
		 */
		static void SRK_CALL createRotationX(float32_t radian, Matrix34& dst);

		/**
		 * direction(LH):(1, 0, 0) to (0, 0, -1)
		 */
		static void SRK_CALL createRotationY(float32_t radian, Matrix34& dst);

		/**
		 * direction(LH):(1, 0, 0) to (0, 1, 0)
		 */
		static void SRK_CALL createRotationZ(float32_t radian, Matrix34& dst);

		static void SRK_CALL createScale(const float32_t(&scale)[3], Matrix34& dst);
		static void SRK_CALL createTranslation(const float32_t(&trans)[3], Matrix34& dst);
		static void SRK_CALL createTRS(const float32_t(trans)[3], const Quaternion* rot, const float32_t(scale)[3], Matrix34& dst);

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
	class SRK_CORE_DLL Matrix44 {
	public:
		using Data34 = float32_t[3][4];
		using Data44 = float32_t[4][4];

		Matrix44();
		Matrix44(const NoInit&);
		Matrix44(
			float32_t m00,       float32_t m01 = 0.f, float32_t m02 = 0.f, float32_t m03 = 0.f,
			float32_t m10 = 0.f, float32_t m11 = 1.f, float32_t m12 = 0.f, float32_t m13 = 0.f,
			float32_t m20 = 0.f, float32_t m21 = 0.f, float32_t m22 = 1.f, float32_t m23 = 0.f,
			float32_t m30 = 0.f, float32_t m31 = 0.f, float32_t m32 = 0.f, float32_t m33 = 1.f);
		Matrix44(const Matrix34& m);
		Matrix44(const Matrix44& m);
		Matrix44(const std::initializer_list<float32_t>& m);
		Matrix44(const float32_t(&m)[3][4]);
		Matrix44(const float32_t(&m)[4][4]);
		~Matrix44();

		static const Matrix44 IDENTITY;

		inline SRK_CALL operator Data44& ();
		inline SRK_CALL operator const Data44& () const;

		void SRK_CALL set33(const Matrix34& m);
		void SRK_CALL set33(const Matrix44& m);
		void SRK_CALL set33(const float32_t(&m)[3][3]);
		void SRK_CALL set33(
			float32_t m00 = 1.f, float32_t m01 = 0.f, float32_t m02 = 0.f,
			float32_t m10 = 0.f, float32_t m11 = 1.f, float32_t m12 = 0.f,
			float32_t m20 = 0.f, float32_t m21 = 0.f, float32_t m22 = 1.f);
		void SRK_CALL set34(const Matrix34& m);
		void SRK_CALL set34(const Matrix44& m);
		void SRK_CALL set34(
			float32_t m00 = 1.f, float32_t m01 = 0.f, float32_t m02 = 0.f, float32_t m03 = 0.f,
			float32_t m10 = 0.f, float32_t m11 = 1.f, float32_t m12 = 0.f, float32_t m13 = 0.f,
			float32_t m20 = 0.f, float32_t m21 = 0.f, float32_t m22 = 1.f, float32_t m23 = 0.f);
		void SRK_CALL set44(const Matrix34& m);
		void SRK_CALL set44(const Matrix44& m);
		void SRK_CALL set44(
			float32_t m00 = 1.f, float32_t m01 = 0.f, float32_t m02 = 0.f, float32_t m03 = 0.f,
			float32_t m10 = 0.f, float32_t m11 = 1.f, float32_t m12 = 0.f, float32_t m13 = 0.f,
			float32_t m20 = 0.f, float32_t m21 = 0.f, float32_t m22 = 1.f, float32_t m23 = 0.f,
			float32_t m30 = 0.f, float32_t m31 = 0.f, float32_t m32 = 0.f, float32_t m33 = 1.f);

		inline void SRK_CALL transpose();
		inline void SRK_CALL transpose(Matrix44& dst) const;

		inline bool SRK_CALL invert();
		inline bool SRK_CALL invert(Matrix44& dst) const;

		inline void SRK_CALL append(const Matrix34& rhs);
		inline void SRK_CALL append(const Matrix44& rhs);
		inline void SRK_CALL append(const Matrix34& rhs, Matrix34& dst) const;
		inline void SRK_CALL append(const Matrix34& rhs, Matrix44& dst) const;
		inline void SRK_CALL append(const Matrix44& rhs, Matrix34& dst) const;
		inline void SRK_CALL append(const Matrix44& rhs, Matrix44& dst) const;

		inline static void SRK_CALL createOrthoLH(float32_t width, float32_t height, float32_t zNear, float32_t zFar, Matrix44& dst) {
			dst.set44(
				2.f / width, 0.f, 0.f, 0.f,
				0.f, 2.f / height, 0.f, 0.f,
				0.f, 0.f, 1.f / (zFar - zNear), zNear / (zNear - zFar));
		}
		inline static Matrix44 SRK_CALL createOrthoLH(float32_t width, float32_t height, float32_t zNear, float32_t zFar) {
			Matrix44 m(NO_INIT);
			createOrthoLH(width, height, zNear, zFar, m);
			return m;
		}
		inline static void SRK_CALL createOrthoOffCenterLH(float32_t left, float32_t right, float32_t bottom, float32_t top, float32_t zNear, float32_t zFar, Matrix44& dst) {
			dst.set44(
				2.f / (right - 1.f), 0.f, 0.f, (1.f + right) / (1.f - right),
				0.f, 2.f / (top - bottom), 0.f, (top + bottom) / (bottom - top),
				0.f, 0.f, 1.f / (zFar - zNear), zNear / (zNear - zFar));
		}
		inline static Matrix44 SRK_CALL createOrthoOffCenterLH(float32_t left, float32_t right, float32_t bottom, float32_t top, float32_t zNear, float32_t zFar) {
			Matrix44 m(NO_INIT);
			createOrthoOffCenterLH(left, right, bottom, top, zNear, zFar, m);
			return m;
		}
		inline static void SRK_CALL createPerspectiveFovLH(float32_t fieldOfViewY, float32_t aspectRatio, float32_t zNear, float32_t zFar, Matrix44& dst) {
			float32_t yScale = 1.f / std::tan(fieldOfViewY * .5f);
			dst.set44(
				yScale / aspectRatio, 0.f, 0.f, 0.f,
				0.f, yScale, 0.f, 0.f,
				0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
				0.f, 0.f, 1.f, 0.f);
		}
		inline static Matrix44 SRK_CALL createPerspectiveFovLH(float32_t fieldOfViewY, float32_t aspectRatio, float32_t zNear, float32_t zFar) {
			Matrix44 m(NO_INIT);
			createPerspectiveFovLH(fieldOfViewY, aspectRatio, zNear, zFar, m);
			return m;
		}
		inline static void SRK_CALL createPerspectiveLH(float32_t width, float32_t height, float32_t zNear, float32_t zFar, Matrix44& dst) {
			auto zNear2 = zNear * 2.f;
			dst.set44(
				zNear2 / width, 0.f, 0.f, 0.f,
				0.f, zNear2 / height, 0.f, 0.f,
				0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
				0.f, 0.f, 1.f, 0.f);
		}
		inline static Matrix44 SRK_CALL createPerspectiveLH(float32_t width, float32_t height, float32_t zNear, float32_t zFar) {
			Matrix44 m(NO_INIT);
			createPerspectiveLH(width, height, zNear, zFar, m);
			return m;
		}
		inline static void SRK_CALL createPerspectiveOffCenterLH(float32_t left, float32_t right, float32_t bottom, float32_t top, float32_t zNear, float32_t zFar, Matrix44& dst) {
			auto zNear2 = zNear * 2.f;
			dst.set44(
				zNear2 / (right - left), 0.f, (left + right) / (left - right), 0.f,
				0.f, zNear2 / (top - bottom), (top + bottom) / (bottom - top), 0.f,
				0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
				0.f, 0.f, 1.f, 0.f);
		}
		inline static Matrix44 SRK_CALL createPerspectiveOffCenterLH(float32_t left, float32_t right, float32_t bottom, float32_t top, float32_t zNear, float32_t zFar) {
			Matrix44 m(NO_INIT);
			createPerspectiveOffCenterLH(left, right, bottom, top, zNear, zFar, m);
			return m;
		}
		//static void SRK_CALL createLookAt(const Vector3& forward, const Vector3& upward, Matrix44& dst);
		//static void SRK_CALL createRotationAxis(const Vector3& axis, f32 radian, Matrix44& dst);

		/**
		 * direction(LH):(0, 1, 0) to (0, 0, 1)
		 */
		 //static void SRK_CALL createRotationX(f32 radian, Matrix44& dst);

		 /**
		  * direction(LH):(1, 0, 0) to (0, 0, -1)
		  */
		  //static void SRK_CALL createRotationY(f32 radian, Matrix44& dst);

		  /**
		   * direction(LH):(1, 0, 0) to (0, 1, 0)
		   */
		   //static void SRK_CALL createRotationZ(f32 radian, Matrix44& dst);

		   //static void SRK_CALL createScale(const Vector3& scale, Matrix44& dst);
		   //static void SRK_CALL createTranslation(const Vector3& trans, Matrix44& dst);
		   //static void SRK_CALL createTRS(const Vector3* trans, const Quaternion* rot, const Vector3* scale, Matrix44& dst);

		union {
			//__m128 col[4];
			Data44 data;
		};
	};


	inline SRK_CALL Matrix34::operator Matrix34::Data& () {
		return data;
	}

	inline SRK_CALL Matrix34::operator const Matrix34::Data& () const {
		return data;
	}

	inline void SRK_CALL Matrix34::transpose(Matrix44& dst) const {
		Math::transposeMat(data, dst.data);
	}

	inline bool SRK_CALL Matrix34::invert() {
		return Math::invertMat(data, data);
	}

	inline bool SRK_CALL Matrix34::invert(Matrix34& dst) const {
		return Math::invertMat(data, dst.data);
	}

	inline bool SRK_CALL Matrix34::invert(Matrix44& dst) const {
		return Math::invertMat(data, ((Matrix34&)dst).data);
	}

	inline void SRK_CALL Matrix34::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void SRK_CALL Matrix34::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void SRK_CALL Matrix34::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void SRK_CALL Matrix34::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void SRK_CALL Matrix34::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void SRK_CALL Matrix34::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void SRK_CALL Matrix34::appendTranslate(const float32_t(&t)[3]) {
		data[0][3] += t[0];
		data[1][3] += t[1];
		data[2][3] += t[2];
	}

	inline void SRK_CALL Matrix34::prependTranslate(const float32_t(&t)[3]) {
		auto x = t[0], y = t[1], z = t[2];
		data[0][3] += x * data[0][0] + y * data[0][1] + z * data[0][2];
		data[1][3] += x * data[1][0] + y * data[1][1] + z * data[1][2];
		data[2][3] += x * data[2][0] + y * data[2][1] + z * data[2][2];
	}

	inline void SRK_CALL Matrix34::setPosition(const float32_t(&p)[3]) {
		setPosition(p[0], p[1], p[2]);
	}

	inline void SRK_CALL Matrix34::setPosition(const Matrix34& m) {
		auto mm = m.data;
		setPosition(mm[0][3], mm[1][3], mm[2][3]);
	}

	inline void SRK_CALL Matrix34::setPosition(const Matrix44& m) {
		auto mm = m.data;
		setPosition(mm[0][3], mm[1][3], mm[2][3]);
	}

	inline void SRK_CALL Matrix34::setPosition(float32_t x, float32_t y, float32_t z) {
		data[0][3] = x;
		data[1][3] = y;
		data[2][3] = z;
	}

	inline void SRK_CALL Matrix34::prependScale(const float32_t(&s)[3]) {
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

	inline SRK_CALL Matrix44::operator Matrix44::Data44& () {
		return data;
	}

	inline SRK_CALL Matrix44::operator const Matrix44::Data44& () const {
		return data;
	}

	inline void SRK_CALL Matrix44::transpose() {
		Math::transposeMat(data, this->data);
	}

	inline void SRK_CALL Matrix44::transpose(Matrix44& dst) const {
		Math::transposeMat(data, dst.data);
	}

	inline bool SRK_CALL Matrix44::invert() {
		return Math::invertMat(data, data);
	}

	inline bool SRK_CALL Matrix44::invert(Matrix44& dst) const {
		return Math::invertMat(data, dst.data);
	}

	inline void SRK_CALL Matrix44::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void SRK_CALL Matrix44::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void SRK_CALL Matrix44::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void SRK_CALL Matrix44::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void SRK_CALL Matrix44::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void SRK_CALL Matrix44::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}
}