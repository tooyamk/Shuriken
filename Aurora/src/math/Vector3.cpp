#include "Vector3.h"
#include "math/Math.h"

namespace aurora {
	Vector3::Vector3() :
		x(0.f),
		y(0.f),
		z(0.f) {
	}

	Vector3::Vector3(const Vector3& v) :
		x(v.x),
		y(v.y),
		z(v.z) {
	}

	Vector3::Vector3(Vector3&& v) :
		x(v.x),
		y(v.y),
		z(v.z) {
	}

	Vector3::Vector3(f32 x, f32 y, f32 z) :
		x(x),
		y(y),
		z(z) {
	}

	Vector3::~Vector3() {
	}

	const Vector3 Vector3::ZERO(0.f, 0.f, 0.f);
	const Vector3 Vector3::ONE(1.f, 1.f, 1.f);

	void Vector3::setNormalize() {
		auto n = x * x + y * y + z * z;
		if (n == 1.f) return;

		n = std::sqrt(n);
		if (n < Math::TOLERANCE<f32>) return;

		n = 1.f / n;

		x *= n;
		y *= n;
		z *= n;
	}

	f32 Vector3::angleBetween(const Vector3& v1, const Vector3& v2) {
		Vector3 n1, n2;
		Vector3::normalize(v1, n1);
		Vector3::normalize(v2, n2);
		auto d = dot(n1, n2);
		if (d > 1.f) {
			d = 1.f;
		} else if (d < -1.f) {
			d = -1.f;
		}

		return std::acos(d);
	}
}