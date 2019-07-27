#include <cmath>
#include "Matrix44.h"
#include "math/Matrix34.h"
#include "math/Quaternion.h"
namespace aurora {
	Matrix44::Matrix44() :
		data{ {1.f, 0.f, 0.f, 0.f},
			 {0.f, 1.f, 0.f, 0.f},
			 {0.f, 0.f, 1.f, 0.f},
			 {0.f, 0.f, 0.f, 1.f} } {
	}

	Matrix44::Matrix44(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23,
		f32 m30, f32 m31, f32 m32, f32 m33) :
		data{ {m00, m01, m02, m03},
			 {m10, m11, m12, m13},
			 {m20, m21, m22, m23},
			 {m30, m31, m32, m33} } {
	}

	Matrix44::Matrix44(const Matrix34& m) : Matrix44(m.data) {
	}

	Matrix44::Matrix44(const Matrix44& m) : Matrix44(m.data) {
	}

	Matrix44::Matrix44(const std::initializer_list<f32>& m) {
		auto p = m.begin();
		if  (uint32_t size = m.size(); size >= 16) {
			auto p = m.begin();
			for (auto e : data) *e = *(p++);
		} else {
			auto m0 = (f32*)data;
			for (uint8_t i = 0; i < size; ++i) m0[i] = *(p++);

			size *= sizeof(f32);
			memset(((uint8_t*)data) + size, 0, sizeof(m) - size);
		}
	}

	Matrix44::Matrix44(const f32(&m)[3][4]) {
		memcpy(data, m, sizeof(m));
		data[3][0] = 0.f;
		data[3][1] = 0.f;
		data[3][2] = 0.f;
		data[3][3] = 1.f;
	}

	Matrix44::Matrix44(const f32(&m)[4][4]) {
		memcpy(data, m, sizeof(m));
	}

	Matrix44::~Matrix44() {
	}

	void Matrix44::set33(const Matrix34& m) {
		memcpy(&data[0][0], &m.data[0][0], sizeof(f32) * 3);
		memcpy(&data[1][0], &m.data[1][0], sizeof(f32) * 3);
		memcpy(&data[2][0], &m.data[2][0], sizeof(f32) * 3);
	}

	void Matrix44::set33(const Matrix44& m) {
		memcpy(&data[0][0], &m.data[0][0], sizeof(f32) * 3);
		memcpy(&data[1][0], &m.data[1][0], sizeof(f32) * 3);
		memcpy(&data[2][0], &m.data[2][0], sizeof(f32) * 3);
	}

	void Matrix44::set33(const f32(&m)[3][3]) {
		memcpy(&data[0][0], &m[0][0], sizeof(f32) * 3);
		memcpy(&data[1][0], &m[1][0], sizeof(f32) * 3);
		memcpy(&data[2][0], &m[2][0], sizeof(f32) * 3);
	}

	void Matrix44::set33(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12,
		f32 m20, f32 m21, f32 m22) {
		data[0][0] = m00;
		data[0][1] = m01;
		data[0][2] = m02;

		data[1][0] = m10;
		data[1][1] = m11;
		data[1][2] = m12;

		data[2][0] = m20;
		data[2][1] = m21;
		data[2][2] = m22;
	}

	void Matrix44::set34(const Matrix34& m) {
		memcpy(data, m.data, sizeof(m.data));
	}

	void Matrix44::set34(const Matrix44& m) {
		memcpy(data, m.data, sizeof(Matrix34));
	}

	void Matrix44::set34(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23) {
		data[0][0] = m00;
		data[0][1] = m01;
		data[0][2] = m02;
		data[0][3] = m03;

		data[1][0] = m10;
		data[1][1] = m11;
		data[1][2] = m12;
		data[1][3] = m13;

		data[2][0] = m20;
		data[2][1] = m21;
		data[2][2] = m22;
		data[2][3] = m23;
	}

	void Matrix44::set44(const Matrix34& m) {
		memcpy(data, m.data, sizeof(m.data));

		data[3][0] = 0.f;
		data[3][1] = 0.f;
		data[3][2] = 0.f;
		data[3][3] = 1.f;
	}

	void Matrix44::set44(const Matrix44& m) {
		memcpy(data, m.data, sizeof(m));
	}

	void Matrix44::set44(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23,
		f32 m30, f32 m31, f32 m32, f32 m33) {
		data[0][0] = m00;
		data[0][1] = m01;
		data[0][2] = m02;
		data[0][3] = m03;

		data[1][0] = m10;
		data[1][1] = m11;
		data[1][2] = m12;
		data[1][3] = m13;

		data[2][0] = m20;
		data[2][1] = m21;
		data[2][2] = m22;
		data[2][3] = m23;

		data[3][0] = m30;
		data[3][1] = m31;
		data[3][2] = m32;
		data[3][3] = m33;
	}

	void Matrix44::createOrthoLH(f32 width, f32 height, f32 zNear, f32 zFar, Matrix44& dst) {
		dst.set44(
			2.f / width, 0.f, 0.f, 0.f,
			0.f, 2.f / height, 0.f, 0.f,
			0.f, 0.f, 1.f / (zFar - zNear), zNear / (zNear - zFar));
	}

	void Matrix44::createOrthoOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar, Matrix44& dst) {
		dst.set44(
			2.f / (right - 1.f), 0.f, 0.f, (1.f + right) / (1.f - right),
			0.f, 2.f / (top - bottom), 0.f, (top + bottom) / (bottom - top),
			0.f, 0.f, 1.f / (zFar - zNear), zNear / (zNear - zFar));
	}

	void Matrix44::createPerspectiveFovLH(f32 fieldOfViewY, f32 aspectRatio, f32 zNear, f32 zFar, Matrix44& dst) {
		f32 yScale = 1.f / std::tan(fieldOfViewY * .5f);
		dst.set44(
			yScale / aspectRatio, 0.f, 0.f, 0.f,
			0.f, yScale, 0.f, 0.f,
			0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
			0.f, 0.f, 1.f, 0.f);
	}

	void Matrix44::createPerspectiveLH(f32 width, f32 height, f32 zNear, f32 zFar, Matrix44& dst) {
		auto zNear2 = zNear * 2.f;
		dst.set44(
			zNear2 / width, 0.f, 0.f, 0.f,
			0.f, zNear2 / height, 0.f, 0.f,
			0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
			0.f, 0.f, 1.f, 0.f);
	}

	void Matrix44::createPerspectiveOffCenterLH(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar, Matrix44& dst) {
		auto zNear2 = zNear * 2.f;
		dst.set44(
			zNear2 / (right - left), 0.f, (left + right) / (left - right), 0.f,
			0.f, zNear2 / (top - bottom), (top + bottom) / (bottom - top), 0.f,
			0.f, 0.f, zFar / (zFar - zNear), zNear * zFar / (zNear - zFar),
			0.f, 0.f, 1.f, 0.f);
	}

	/*
	void Matrix44::createLookAt(const Vector3& forward, const Vector3& upward, Matrix44& dst) {
		auto& zaxis = forward;
		Vector3 xaxis, yaxis;
		Vector3::cross(upward, zaxis, xaxis);
		xaxis.setNormalize();
		Vector3::cross(zaxis, xaxis, yaxis);

		dst.set44(
			xaxis.x, yaxis.x, zaxis.x, 0.f,
			xaxis.x, yaxis.y, zaxis.y, 0.f,
			xaxis.x, yaxis.z, zaxis.z);
	}

	void Matrix44::createRotationAxis(const Vector3& axis, f32 radian, Matrix44& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);
		auto cos1 = 1.f - cos;
		auto cos1x = cos1 * axis.x;
		auto cos1xy = cos1x * axis.y;
		auto cos1xz = cos1x * axis.z;
		auto cos1y = cos1 * axis.y;
		auto cos1yz = cos1y * axis.z;
		auto xsin = axis.x * sin;
		auto ysin = axis.y * sin;
		auto zsin = axis.z * sin;

		dst.set44(
			cos + cos1x * axis.x, cos1xy + zsin, cos1xz - ysin, 0.f,
			cos1xy - zsin, cos + cos1y * axis.y, cos1yz + xsin, 0.f,
			cos1xz + ysin, cos1yz - xsin, cos + cos1 * axis.z * axis.z);
	}

	void Matrix44::createRotationX(f32 radian, Matrix44& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set44(
			1.f, 0.f, 0.f, 0.f,
			0.f, cos, -sin, 0.f,
			0.f, sin, cos);
	}

	void Matrix44::createRotationY(f32 radian, Matrix44& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set44(
			cos, 0.f, sin, 0.f,
			0.f, 1.f, 0.f, 0.f,
			-sin, 0.f, cos);
	}

	void Matrix44::createRotationZ(f32 radian, Matrix44& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set44(
			cos, -sin, 0.f, 0.f,
			sin, cos);
	}

	void Matrix44::createScale(const Vector3& scale, Matrix44& dst) {
		dst.set44(
			scale.x, 0.f, 0.f, 0.f,
			0.f, scale.y, 0.f, 0.f,
			0.f, 0.f, scale.z);
	}

	void Matrix44::createTranslation(const Vector3& trans, Matrix44& dst) {
		dst.set44(
			1.f, 0.f, 0.f, trans.x,
			0.f, 1.f, 0.f, trans.y,
			0.f, 0.f, 1.f, trans.z);
	}

	void Matrix44::createTRS(const Vector3* trans, const Quaternion* rot, const Vector3* scale, Matrix44& dst) {
		if (rot) {
			rot->toMatrix33(dst);
		} else {
			dst.set33();
		}

		auto& m = dst.m44;

		m[3][0] = 0.f;
		m[3][1] = 0.f;
		m[3][2] = 0.f;
		m[3][3] = 1.f;

		if (scale) {
			dst.mul(*scale);
			rst.m00 *= scale.x;
			rst.m01 *= scale.x;
			rst.m02 *= scale.x;

			rst.m10 *= scale.y;
			rst.m11 *= scale.y;
			rst.m12 *= scale.y;

			rst.m20 *= scale.z;
			rst.m21 *= scale.z;
			rst.m22 *= scale.z;
		}

		if (trans) {
			m[0][3] = trans->x;
			m[1][3] = trans->y;
			m[2][3] = trans->z;
		} else {
			m[0][3] = 0.f;
			m[1][3] = 0.f;
			m[2][3] = 0.f;
		}
	}
	*/
}
