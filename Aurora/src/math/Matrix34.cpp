#include <cmath>
#include <string>
#include "Matrix34.h"
#include "math/Matrix44.h"
#include "math/Quaternion.h"

namespace aurora {
	Matrix34::Matrix34() :
		data{ {1.f, 0.f, 0.f, 0.f},
			 {0.f, 1.f, 0.f, 0.f},
			 {0.f, 0.f, 1.f, 0.f} } {
	}

	Matrix34::Matrix34(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23) :
		data{ {m00, m01, m02, m03},
			 {m10, m11, m12, m13},
			 {m20, m21, m22, m23} } {
	}

	Matrix34::Matrix34(const Matrix34& m) : Matrix34(m.data) {
	}

	Matrix34::Matrix34(const Matrix44& m) : Matrix34(m.data) {
	}

	Matrix34::Matrix34(const std::initializer_list<f32>& m) {
		auto p = m.begin();
		auto size = m.size();
		if (size >= 12) {
			auto p = m.begin();
			for (auto e : data) *e = *(p++);
		} else {
			auto m0 = (f32*)data;
			for (ui8 i = 0; i < size; ++i) m0[i] = *(p++);

			size *= sizeof(f32);
			memset(((i8*)data) + size, 0, sizeof(m) - size);
		}
	}

	Matrix34::Matrix34(const f32(&m)[3][4]) {
		memcpy(data, m, sizeof(m));
	}

	Matrix34::Matrix34(const f32(&m)[4][4]) {
		memcpy(data, m, sizeof(data));
	}

	Matrix34::~Matrix34() {
	}

	void Matrix34::set33(const Matrix34& m) {
		memcpy(&data[0][0], &m.data[0][0], sizeof(f32) * 3);
		memcpy(&data[1][0], &m.data[1][0], sizeof(f32) * 3);
		memcpy(&data[2][0], &m.data[2][0], sizeof(f32) * 3);
	}

	void Matrix34::set33(const Matrix44& m) {
		memcpy(&data[0][0], &m.data[0][0], sizeof(f32) * 3);
		memcpy(&data[1][0], &m.data[1][0], sizeof(f32) * 3);
		memcpy(&data[2][0], &m.data[2][0], sizeof(f32) * 3);
	}

	void Matrix34::set33(const f32(&m)[3][3]) {
		memcpy(&data[0][0], &m[0][0], sizeof(f32) * 3);
		memcpy(&data[1][0], &m[1][0], sizeof(f32) * 3);
		memcpy(&data[2][0], &m[2][0], sizeof(f32) * 3);
	}

	void Matrix34::set33(
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

	void Matrix34::set34(const Matrix34& m) {
		memcpy(data, m.data, sizeof(data));
	}

	void Matrix34::set34(const Matrix44& m) {
		memcpy(data, m.data, sizeof(data));
	}

	void Matrix34::set34(
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

	void Matrix34::decomposition(Matrix34* dstRot, f32(dstScale[3])) const {
		auto& m = data;

		f32 d[3][3];

		auto x = m[0][0], y = m[1][0], z = m[2][0];
		d[0][0] = x;
		d[1][0] = y;
		d[2][0] = z;

		auto dot = x * x + y * y + z * z;
		if (dot != 1.f) {
			dot = std::sqrt(dot);
			if (dot > Math::TOLERANCE<f32>) {
				dot = 1.f / dot;

				d[0][0] *= dot;
				d[1][0] *= dot;
				d[2][0] *= dot;
			}
		}

		x = m[0][1], y = m[1][1], z = m[2][1];
		dot = d[0][0] * x + d[1][0] * y + d[2][0] * z;
		x -= d[0][0] * dot;
		y -= d[1][0] * dot;
		z -= d[2][0] * dot;

		d[0][1] = x;
		d[1][1] = y;
		d[2][1] = z;

		dot = x * x + y * y + z * z;
		if (dot != 1.f) {
			dot = std::sqrt(dot);
			if (dot > Math::TOLERANCE<f32>) {
				dot = 1.f / dot;

				d[0][1] *= dot;
				d[1][1] *= dot;
				d[2][1] *= dot;
			}
		}

		x = m[0][2], y = m[1][2], z = m[2][2];
		dot = d[0][0] * x + d[1][0] * y + d[2][0] * z;
		d[0][2] = x - d[0][0] * dot;
		d[1][2] = y - d[1][0] * dot;
		d[2][2] = z - d[2][0] * dot;

		dot = d[0][1] * x + d[1][1] * y + d[2][1] * z;
		d[0][2] -= d[0][1] * dot;
		d[1][2] -= d[1][1] * dot;
		d[2][2] -= d[2][1] * dot;

		dot = d[0][2] * x + d[1][2] * y + d[2][2] * z;
		if (dot != 1.f) {
			dot = std::sqrt(dot);
			if (dot > Math::TOLERANCE<f32>) {
				dot = 1.f / dot;

				d[0][2] *= dot;
				d[1][2] *= dot;
				d[2][2] *= dot;
			}
		}

		dot = d[0][0] * d[1][1] * d[2][2] +
			d[0][1] * d[1][2] * d[2][0] +
			d[0][2] * d[1][0] * d[2][1] -
			d[0][2] * d[1][1] * d[2][0] -
			d[0][1] * d[1][0] * d[2][2] -
			d[0][0] * d[1][2] * d[2][1];

		if (dot < 0.f) {
			d[0][0] = -d[0][0];
			d[0][1] = -d[0][1];
			d[0][2] = -d[0][2];
			d[1][0] = -d[1][0];
			d[1][1] = -d[1][1];
			d[1][2] = -d[1][2];
			d[2][0] = -d[2][0];
			d[2][1] = -d[2][1];
			d[2][2] = -d[2][2];
		}

		if (dstRot) {
			dstRot->set33(d);
			auto& mm = dstRot->data;
			mm[0][3] = 0.f;
			mm[1][3] = 0.f;
			mm[2][3] = 0.f;
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

		auto tr = m[0][0] + m[1][1] + m[2][2];
		if (tr > 0.f) {
			auto s = std::sqrt(tr + 1.f);
			dst.w = s * .5f;
			s = .5f / s;
			dst.x = (m[2][1] - m[1][2]) * s;
			dst.y = (m[0][2] - m[2][0]) * s;
			dst.z = (m[1][0] - m[0][1]) * s;
		} else {
			if (m[1][1] > m[0][0]) {
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
	}

	void Matrix34::createLookAt(const f32(&forward)[3], const f32(&upward)[3], Matrix34& dst) {
		auto& zaxis = forward;
		f32 xaxis[3], yaxis[3];
		Math::cross<f32, f32>(upward, zaxis, xaxis);
		Math::normalize<3, f32, f32>(xaxis, xaxis);
		Math::cross<f32, f32>(zaxis, xaxis, yaxis);

		dst.set34(
			xaxis[0], yaxis[0], zaxis[0], 0.f,
			xaxis[1], yaxis[1], zaxis[1], 0.f,
			xaxis[2], yaxis[2], zaxis[2]);
	}

	void Matrix34::createRotationAxis(const f32(&axis)[3], f32 radian, Matrix34& dst) {
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

	void Matrix34::createRotationX(f32 radian, Matrix34& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set34(
			1.f, 0.f, 0.f, 0.f,
			0.f, cos, -sin, 0.f,
			0.f, sin, cos);
	}

	void Matrix34::createRotationY(f32 radian, Matrix34& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set34(
			cos, 0.f, sin, 0.f,
			0.f, 1.f, 0.f, 0.f,
			-sin, 0.f, cos);
	}

	void Matrix34::createRotationZ(f32 radian, Matrix34& dst) {
		auto sin = std::sin(radian);
		auto cos = std::cos(radian);

		dst.set34(
			cos, -sin, 0.f, 0.f,
			sin, cos);
	}

	void Matrix34::createScale(const f32(&scale)[3], Matrix34& dst) {
		dst.set34(
			scale[0], 0.f, 0.f, 0.f,
			0.f, scale[1], 0.f, 0.f,
			0.f, 0.f, scale[2]);
	}

	void Matrix34::createTranslation(const f32(&trans)[3], Matrix34& dst) {
		dst.set34(
			1.f, 0.f, 0.f, trans[0],
			0.f, 1.f, 0.f, trans[1],
			0.f, 0.f, 1.f, trans[2]);
	}

	void Matrix34::createTRS(const f32(&trans)[3], const Quaternion* rot, const f32(&scale)[3], Matrix34& dst) {
		if (rot) {
			rot->toMatrix(dst);
		} else {
			dst.set33();
		}

		auto& m = dst.data;

		m[3][0] = 0.f;
		m[3][1] = 0.f;
		m[3][2] = 0.f;
		m[3][3] = 1.f;

		if (scale) dst.prependScale(scale);

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
}
