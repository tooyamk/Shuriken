#include "Quaternion.h"
#include "math/Math.h"
#include "math/Matrix34.h"
#include "math/Matrix44.h"

namespace aurora {
	Quaternion::Quaternion() :
		x(0.f),
		y(0.f),
		z(0.f),
		w(1.f) {
	}

	Quaternion::Quaternion(const Quaternion& q) :
		x(q.x),
		y(q.y),
		z(q.z),
		w(q.w) {
	}

	Quaternion::Quaternion(Quaternion&& q) :
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

	const Quaternion Quaternion::IDENTITY(0.f, 0.f, 0.f, 1.f);

	void Quaternion::normalize() {
		auto n = x * x + y * y + z * z + w * w;
		if (n == 1.f) return;

		n = std::sqrt(n);
		if (n < Math::TOLERANCE<f32>) return;

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

	void Quaternion::createFromEulerX(f32 radian, Quaternion& dst) {
		radian *= .5f;
		auto x = std::sin(radian);
		auto w = std::cos(radian);

		dst.set(x, 0.f, 0.f, w);
	}

	void Quaternion::createFromEulerY(f32 radian, Quaternion& dst) {
		radian *= .5f;
		auto y = std::sin(radian);
		auto w = std::cos(radian);

		dst.set(0.f, y, 0.f, w);
	}

	void Quaternion::createFromEulerZ(f32 radian, Quaternion& dst) {
		radian *= .5f;
		auto z = std::sin(radian);
		auto w = std::cos(radian);

		dst.set(0.f, 0.f, z, w);
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