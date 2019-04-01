#include "Vector2.h"
#include "math/Math.h"

namespace aurora {
	Vector2::Vector2() :
		x(0.f),
		y(0.f) {
	}

	Vector2::Vector2(const Vector2& v) :
		x(v.x),
		y(v.y) {
	}

	Vector2::Vector2(Vector2&& v) :
		x(v.x),
		y(v.y) {
	}

	Vector2::Vector2(f32 x, f32 y) :
		x(x),
		y(y) {
	}

	Vector2::~Vector2() {
	}

	const Vector2 Vector2::ZERO(0.f, 0.f);
	const Vector2 Vector2::ONE(1.f, 1.f);

	void Vector2::setNormalize() {
		auto n = x * x + y * y;
		if (n == 1.f) return;

		n = std::sqrt(n);
		if (n < Math::TOLERANCE<f32>) return;

		n = 1.f / n;

		x *= n;
		y *= n;
	}

	f32 Vector2::angleBetween(const Vector2& v1, const Vector2& v2) {
		Vector2 n1, n2;
		Vector2::normalize(v1, n1);
		Vector2::normalize(v2, n2);
		auto d = dot(n1, n2);
		if (d > 1.f) {
			d = 1.f;
		} else if (d < -1.f) {
			d = -1.f;
		}

		return std::acos(d);
	}
}