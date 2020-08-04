#include <cmath>
#include "Matrix.h"
#include "aurora/math/Quaternion.h"

namespace aurora {
	Matrix34::Matrix34() :
		data{ {1.f, 0.f, 0.f, 0.f},
			  {0.f, 1.f, 0.f, 0.f},
			  {0.f, 0.f, 1.f, 0.f} } {
	}

	Matrix34::Matrix34(const NoInit&) {}

	Matrix34::Matrix34(
		float32_t m00, float32_t m01, float32_t m02, float32_t m03,
		float32_t m10, float32_t m11, float32_t m12, float32_t m13,
		float32_t m20, float32_t m21, float32_t m22, float32_t m23) :
		data{ {m00, m01, m02, m03},
			 {m10, m11, m12, m13},
			 {m20, m21, m22, m23} } {
	}

	Matrix34::Matrix34(const Matrix34& m) : Matrix34(m.data) {
	}

	Matrix34::Matrix34(const Matrix44& m) : Matrix34(m.data) {
	}

	Matrix34::Matrix34(const std::initializer_list<float32_t>& m) {
		auto p = m.begin();
		if (auto size = m.size(); size >= 12) {
			for (auto e : data) *e = *(p++);
		} else {
			auto m0 = (float32_t*)data;
			for (uint8_t i = 0; i < size; ++i) m0[i] = *(p++);

			size *= sizeof(float32_t);
			memset(((uint8_t*)data) + size, 0, sizeof(m) - size);
		}
	}

	Matrix34::Matrix34(const float32_t(&m)[3][4]) {
		memcpy(data, m, sizeof(data));
	}

	Matrix34::Matrix34(const float32_t(&m)[4][4]) {
		memcpy(data, m, sizeof(data));
	}

	Matrix34::~Matrix34() {
	}

	const Matrix34 Matrix34::IDENTITY = Matrix34();

	void Matrix34::set33(const Matrix34& m) {
		for (size_t i = 0; i < 3; ++i) memcpy(&data[i][0], &m.data[i][0], sizeof(float32_t) * 3);
	}

	void Matrix34::set33(const Matrix44& m) {
		for (size_t i = 0; i < 3; ++i) memcpy(&data[i][0], &m.data[i][0], sizeof(float32_t) * 3);
	}

	void Matrix34::set33(const float32_t(&m)[3][3]) {
		for (size_t i = 0; i < 3; ++i) memcpy(&data[i][0], &m[i][0], sizeof(float32_t) * 3);
	}

	void Matrix34::set33(
		float32_t m00, float32_t m01, float32_t m02,
		float32_t m10, float32_t m11, float32_t m12,
		float32_t m20, float32_t m21, float32_t m22) {
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

	void Matrix34::set34(const Matrix34& m) {
		memcpy(data, m.data, sizeof(data));
	}

	void Matrix34::set34(const Matrix44& m) {
		memcpy(data, m.data, sizeof(data));
	}

	void Matrix34::set34(
		float32_t m00, float32_t m01, float32_t m02, float32_t m03,
		float32_t m10, float32_t m11, float32_t m12, float32_t m13,
		float32_t m20, float32_t m21, float32_t m22, float32_t m23) {
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

	void Matrix34::decomposition(Matrix34* dstRot, float32_t(dstScale[3])) const {
		auto& m = data;

		float32_t d[3][3];

		float32_t xyz[3] = { m[0][0], m[0][0], m[0][0] };
		for (size_t i = 0; i < 3; ++i) d[i][0] = xyz[i];

		auto dot = xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2];
		if (dot != 1.f) {
			if (dot = std::sqrt(dot); dot > Math::TOLERANCE<float32_t>) {
				dot = 1.f / dot;
				for (size_t i = 0; i < 3; ++i) d[i][0] *= dot;
			}
		}

		for (size_t i = 0; i < 3; ++i) xyz[i] = m[i][1];
		dot = d[0][0] * xyz[0] + d[1][0] * xyz[1] + d[2][0] * xyz[2];
		for (size_t i = 0; i < 3; ++i) {
			xyz[i] -= d[i][0] * dot;
			d[i][1] = xyz[i];
		}

		dot = xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2];
		if (dot != 1.f) {
			if (dot = std::sqrt(dot); dot > Math::TOLERANCE<float32_t>) {
				dot = 1.f / dot;
				for (size_t i = 0; i < 3; ++i) d[i][1] *= dot;
			}
		}

		for (size_t i = 0; i < 3; ++i) xyz[i] = m[i][2];
		dot = d[0][0] * xyz[0] + d[1][0] * xyz[1] + d[2][0] * xyz[2];
		for (size_t i = 0; i < 3; ++i) d[i][2] = xyz[i] - d[i][0] * dot;

		dot = d[0][1] * xyz[0] + d[1][1] * xyz[1] + d[2][1] * xyz[2];
		for (size_t i = 0; i < 3; ++i) d[i][2] -= d[i][1] * dot;

		dot = d[0][2] * xyz[0] + d[1][2] * xyz[1] + d[2][2] * xyz[2];
		if (dot != 1.f) {
			if (dot = std::sqrt(dot); dot > Math::TOLERANCE<float32_t>) {
				dot = 1.f / dot;
				for (size_t i = 0; i < 3; ++i) d[i][2] *= dot;
			}
		}

		dot = d[0][0] * d[1][1] * d[2][2] +
			  d[0][1] * d[1][2] * d[2][0] +
			  d[0][2] * d[1][0] * d[2][1] -
			  d[0][2] * d[1][1] * d[2][0] -
			  d[0][1] * d[1][0] * d[2][2] -
			  d[0][0] * d[1][2] * d[2][1];

		if (dot < 0.f) {
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) d[i][j] = -d[i][j];
			}
		}

		if (dstRot) {
			dstRot->set33(d);
			auto& mm = dstRot->data;
			for (size_t i = 0; i < 3; ++i) mm[i][3] = 0.f;
		}

		if (dstScale) {
			auto x = d[0][0] * m[0][0] + d[1][0] * m[1][0] + d[2][0] * m[2][0];
			auto y = d[0][1] * m[0][1] + d[1][1] * m[1][1] + d[2][1] * m[2][1];
			auto z = d[0][2] * m[0][2] + d[1][2] * m[1][2] + d[2][2] * m[2][2];
			dstScale[0] = x;
			dstScale[1] = y;
			dstScale[2] = z;
		}
	}

	void Matrix34::toQuaternion(Quaternion& dst) const {
		auto& m = data;

		if (auto tr = m[0][0] + m[1][1] + m[2][2]; tr > 0.f) {
			auto s = std::sqrt(tr + 1.f);
			dst.w = s * .5f;
			s = .5f / s;
			dst.x = (m[2][1] - m[1][2]) * s;
			dst.y = (m[0][2] - m[2][0]) * s;
			dst.z = (m[1][0] - m[0][1]) * s;
		} else if (m[1][1] > m[0][0]) {
			if (m[2][2] > m[1][1]) {//2
				auto s = std::sqrt(m[2][2] - m[0][0] - m[1][1] + 1.f);
				dst.z = s * 0.5f;
				s = .5f / s;
				dst.x = (m[0][2] + m[2][0]) * s;
				dst.y = (m[1][2] + m[2][1]) * s;
				dst.w = (m[1][0] - m[0][1]) * s;
			} else {//1
				auto s = std::sqrt(m[1][1] - m[2][2] - m[0][0] + 1.f);
				dst.y = s * .5f;
				s = .5f / s;
				dst.x = (m[0][1] + m[1][0]) * s;
				dst.z = (m[2][1] + m[1][2]) * s;
				dst.w = (m[0][2] - m[2][0]) * s;
			}
		} else if (m[2][2] > m[0][0]) {//2
			auto s = std::sqrt(m[2][2] - m[0][0] - m[1][1] + 1.f);
			dst.z = s * .5f;
			s = .5f / s;
			dst.x = (m[0][2] + m[2][0]) * s;
			dst.y = (m[1][2] + m[2][1]) * s;
			dst.w = (m[1][0] - m[0][1]) * s;
		} else {//0
			auto s = std::sqrt(m[0][0] - m[1][1] - m[2][2] + 1.f);
			dst.x = s * .5f;
			s = .5f / s;
			dst.y = (m[1][0] + m[0][1]) * s;
			dst.z = (m[2][0] + m[0][2]) * s;
			dst.w = (m[2][1] - m[1][2]) * s;
		}
	}

	void Matrix34::createLookAt(const float32_t(&forward)[3], const float32_t(&upward)[3], Matrix34& dst) {
		auto& zaxis = forward;
		float32_t xaxis[3], yaxis[3];
		Math::cross<float32_t, float32_t>(upward, zaxis, xaxis);
		Math::normalize(xaxis, 1.0f);
		Math::cross<float32_t, float32_t>(zaxis, xaxis, yaxis);

		dst.set34(
			xaxis[0], yaxis[0], zaxis[0], 0.f,
			xaxis[1], yaxis[1], zaxis[1], 0.f,
			xaxis[2], yaxis[2], zaxis[2]);
	}

	void Matrix34::createRotationAxis(const float32_t(&axis)[3], float32_t radian, Matrix34& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);
		auto cos1 = 1.f - cos;
		auto cos1x = cos1 * axis[0];
		auto cos1xy = cos1x * axis[1];
		auto cos1xz = cos1x * axis[2];
		auto cos1y = cos1 * axis[1];
		auto cos1yz = cos1y * axis[2];
		auto xsin = axis[0] * sin;
		auto ysin = axis[1] * sin;
		auto zsin = axis[2] * sin;

		dst.set34(
			cos + cos1x * axis[0], cos1xy + zsin, cos1xz - ysin, 0.f,
			cos1xy - zsin, cos + cos1y * axis[1], cos1yz + xsin, 0.f,
			cos1xz + ysin, cos1yz - xsin, cos + cos1 * axis[2] * axis[2]);
	}

	void Matrix34::createRotationX(float32_t radian, Matrix34& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set34(
			1.f, 0.f, 0.f, 0.f,
			0.f, cos, -sin, 0.f,
			0.f, sin, cos);
	}

	void Matrix34::createRotationY(float32_t radian, Matrix34& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set34(
			cos, 0.f, sin, 0.f,
			0.f, 1.f, 0.f, 0.f,
			-sin, 0.f, cos);
	}

	void Matrix34::createRotationZ(float32_t radian, Matrix34& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set34(
			cos, -sin, 0.f, 0.f,
			sin, cos);
	}

	void Matrix34::createScale(const float32_t(&scale)[3], Matrix34& dst) {
		dst.set34(
			scale[0], 0.f, 0.f, 0.f,
			0.f, scale[1], 0.f, 0.f,
			0.f, 0.f, scale[2]);
	}

	void Matrix34::createTranslation(const float32_t(&trans)[3], Matrix34& dst) {
		dst.set34(
			1.f, 0.f, 0.f, trans[0],
			0.f, 1.f, 0.f, trans[1],
			0.f, 0.f, 1.f, trans[2]);
	}

	void Matrix34::createTRS(const float32_t(trans)[3], const Quaternion* rot, const float32_t(scale)[3], Matrix34& dst) {
		if (rot) {
			rot->toMatrix(dst);
		} else {
			dst.set33();
		}

		auto& m = dst.data;

		if (scale) dst.prependScale((const float32_t(&)[3])scale);

		if (trans) {
			m[0][3] = trans[0];
			m[1][3] = trans[1];
			m[2][3] = trans[2];
		} else {
			m[0][3] = 0.f;
			m[1][3] = 0.f;
			m[2][3] = 0.f;
		}
	}


	Matrix44::Matrix44() :
		data{ {1.f, 0.f, 0.f, 0.f},
			  {0.f, 1.f, 0.f, 0.f},
			  {0.f, 0.f, 1.f, 0.f},
		 	  {0.f, 0.f, 0.f, 1.f} } {
	}

	Matrix44::Matrix44(const NoInit&) {}

	Matrix44::Matrix44(
		float32_t m00, float32_t m01, float32_t m02, float32_t m03,
		float32_t m10, float32_t m11, float32_t m12, float32_t m13,
		float32_t m20, float32_t m21, float32_t m22, float32_t m23,
		float32_t m30, float32_t m31, float32_t m32, float32_t m33) :
		data{ {m00, m01, m02, m03},
			  {m10, m11, m12, m13},
			  {m20, m21, m22, m23},
			  {m30, m31, m32, m33} } {
	}

	Matrix44::Matrix44(const Matrix34& m) : Matrix44(m.data) {
	}

	Matrix44::Matrix44(const Matrix44& m) : Matrix44(m.data) {
	}

	Matrix44::Matrix44(const std::initializer_list<float32_t>& m) {
		auto p = m.begin();
		if (uint32_t size = m.size(); size >= 16) {
			auto p = m.begin();
			for (auto e : data) *e = *(p++);
		} else {
			auto m0 = (float32_t*)data;
			for (uint8_t i = 0; i < size; ++i) m0[i] = *(p++);

			size *= sizeof(float32_t);
			memset(((uint8_t*)data) + size, 0, sizeof(m) - size);
		}
	}

	Matrix44::Matrix44(const float32_t(&m)[3][4]) {
		memcpy(data, m, sizeof(m));
		data[3][0] = 0.f;
		data[3][1] = 0.f;
		data[3][2] = 0.f;
		data[3][3] = 1.f;
	}

	Matrix44::Matrix44(const float32_t(&m)[4][4]) {
		memcpy(data, m, sizeof(m));
	}

	Matrix44::~Matrix44() {
	}

	const Matrix44 Matrix44::IDENTITY = Matrix44();

	void Matrix44::set33(const Matrix34& m) {
		for (size_t i = 0; i < 3; ++i) memcpy(&data[i][0], &m.data[i][0], sizeof(float32_t) * 3);
	}

	void Matrix44::set33(const Matrix44& m) {
		for (size_t i = 0; i < 3; ++i) memcpy(&data[i][0], &m.data[i][0], sizeof(float32_t) * 3);
	}

	void Matrix44::set33(const float32_t(&m)[3][3]) {
		for (size_t i = 0; i < 3; ++i) memcpy(&data[i][0], &m[i][0], sizeof(float32_t) * 3);
	}

	void Matrix44::set33(
		float32_t m00, float32_t m01, float32_t m02,
		float32_t m10, float32_t m11, float32_t m12,
		float32_t m20, float32_t m21, float32_t m22) {
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
		float32_t m00, float32_t m01, float32_t m02, float32_t m03,
		float32_t m10, float32_t m11, float32_t m12, float32_t m13,
		float32_t m20, float32_t m21, float32_t m22, float32_t m23) {
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
		float32_t m00, float32_t m01, float32_t m02, float32_t m03,
		float32_t m10, float32_t m11, float32_t m12, float32_t m13,
		float32_t m20, float32_t m21, float32_t m22, float32_t m23,
		float32_t m30, float32_t m31, float32_t m32, float32_t m33) {
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
}
