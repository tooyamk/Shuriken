#include "Quaternion.h"
#include "aurora/math/Math.h"
#include "aurora/math/Matrix.h"

namespace aurora {
	Quaternion::Quaternion() :
		x(0.f),
		y(0.f),
		z(0.f),
		w(1.f) {
	}

	Quaternion::Quaternion(const NoInit&) {}

	Quaternion::Quaternion(const Quaternion& q) :
		x(q.x),
		y(q.y),
		z(q.z),
		w(q.w) {
	}

	Quaternion::Quaternion(Quaternion&& q) noexcept :
		x(q.x),
		y(q.y),
		z(q.z),
		w(q.w) {
	}

	Quaternion::Quaternion(f32 x, f32 y, f32 z, f32 w) :
		x(x),
		y(y),
		z(z),
		w(w) {
	}

	Quaternion::~Quaternion() {
	}

	const Quaternion Quaternion::IDENTITY = Quaternion();

	void Quaternion::normalize() {
		auto n = x * x + y * y + z * z + w * w;

		if (n == 1.f) return;
		if (n = std::sqrt(n); n < Math::TOLERANCE<f32>) return;

		n = 1.f / n;

		x *= n;
		y *= n;
		z *= n;
		w *= n;
	}

	void Quaternion::toEuler(f32(&dst)[3]) const {
		auto y2 = y * y;
		auto ex = std::atan2(2.f * (w * x + y * z), (1.f - 2.f * (x * x + y2)));
		auto ey = std::asin(2.f * (w * y - z * x));
		auto ez = std::atan2(2.f * (w * z + x * y), (1.f - 2.f * (y2 + z * z)));
		dst[0] = ex;
		dst[1] = ey;
		dst[2] = ez;
	}

	void Quaternion::toMatrix(Matrix34& dst) const {
		auto x2 = x * 2.f, y2 = y * 2.f, z2 = z * 2.f;
		auto xx = x * x2;
		auto xy = x * y2;
		auto xz = x * z2;
		auto yy = y * y2;
		auto yz = y * z2;
		auto zz = z * z2;
		auto wx = w * x2;
		auto wy = w * y2;
		auto wz = w * z2;

		auto& m = dst.data;
		m[0][0] = 1.f - yy - zz;
		m[1][0] = xy + wz;
		m[2][0] = xz - wy;

		m[0][1] = xy - wz;
		m[1][1] = 1.f - xx - zz;
		m[2][1] = yz + wx;

		m[0][2] = xz + wy;
		m[1][2] = yz - wx;
		m[2][2] = 1.f - xx - yy;
	}

	void Quaternion::createFromEuler(const f32(&radians)[3], Quaternion& dst) {
		auto x = radians[0] * .5f;
		auto y = radians[1] * .5f;
		auto z = radians[2] * .5f;

		auto sx = std::sin(x);
		auto cx = std::cos(x);
		auto sy = std::sin(y);
		auto cy = std::cos(y);
		auto sz = std::sin(z);
		auto cz = std::cos(z);

		auto sxcy = sx * cy;
		auto cxsy = cx * sy;
		auto cxcy = cx * cy;
		auto sxsy = sx * sy;

		dst.set(sxcy * cz - cxsy * sz, cxsy * cz + sxcy * sz, cxcy * sz - sxsy * cz, cxcy * cz + sxsy * sz);
	}

	void Quaternion::createFromAxis(const f32(&axis)[3], f32 radian, Quaternion& dst) {
		radian *= .5f;
		auto s = std::sin(radian);

		dst.set(-axis[0] * s, -axis[1] * s, -axis[2] * s, std::cos(radian));
	}

	void Quaternion::createLookAt(const f32(&forward)[3], const f32(&upward)[3], Quaternion& dst) {
		auto& zaxis = forward;
		f32 xaxis[3], yaxis[3];
		Math::cross<f32, f32>(upward, zaxis, xaxis);
		Math::normalize<3, f32, f32>(xaxis, xaxis);
		Math::cross<f32, f32>(zaxis, xaxis, yaxis);

		auto w = std::sqrt(1.f + xaxis[0] + yaxis[1] + zaxis[2]) * .5f;
		auto recip = .25f / w;

		dst.set((yaxis[2] - zaxis[1]) * recip, (zaxis[0] - xaxis[2]) * recip, (xaxis[1] - yaxis[0]) * recip, w);
	}
}