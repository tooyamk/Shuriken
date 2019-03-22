#include "Quaternion.h"
#include "math/Math.h"
#include "math/Matrix34.h"
#include "math/Matrix44.h"
#include "math/Vector3.h"

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

	void Quaternion::toEuler(Vector3& dst) const {
		auto y2 = y * y;
		dst.x = std::atan2(2.f * (w * x + y * z), (1.f - 2.f * (x * x + y2)));
		dst.y = std::asin(2.f * (w * y - z * x));
		dst.z = std::atan2(2.f * (w * z + x * y), (1.f - 2.f * (y2 + z * z)));
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

	void Quaternion::createFromEuler(const Vector3& radians, Quaternion& dst) {
		auto x = radians.x * .5f;
		auto y = radians.y * .5f;
		auto z = radians.z * .5f;

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

	void Quaternion::createFromAxis(const Vector3& axis, f32 radian, Quaternion& dst) {
		radian *= .5f;
		auto s = std::sin(radian);

		dst.set(-axis.x * s, -axis.y * s, -axis.z * s, std::cos(radian));
	}

	void Quaternion::createLookAt(const Vector3& forward, const Vector3& upward, Quaternion& dst) {
		auto& zaxis = forward;
		Vector3 xaxis, yaxis;
		Vector3::cross(upward, zaxis, xaxis);
		xaxis.setNormalize();
		Vector3::cross(zaxis, xaxis, yaxis);

		auto w = std::sqrt(1.f + xaxis.x + yaxis.y + zaxis.z) * .5f;
		auto recip = .25f / w;

		dst.set((yaxis.z - zaxis.y) * recip, (zaxis.x - xaxis.z) * recip, (xaxis.y - yaxis.x) * recip, w);
	}
}