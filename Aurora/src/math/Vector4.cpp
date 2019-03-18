#include "Vector4.h"
#include "math/Math.h"

namespace aurora {
	Vector4::Vector4() :
		x(0.f),
		y(0.f),
		z(0.f),
		w(0.f) {
	}

	Vector4::Vector4(const Vector4& v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w) {
	}

	Vector4::Vector4(Vector4&& v) :
		x(v.x),
		y(v.y),
		z(v.z),
		w(v.w) {
	}

	Vector4::Vector4(f32 x, f32 y, f32 z, f32 w) :
		x(x),
		y(y),
		z(z),
		w(w) {
	}

	Vector4::~Vector4() {
	}

	const Vector4 Vector4::ZERO(0.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::ONE(1.f, 1.f, 1.f, 1.f);

	void Vector4::setNormalize() {
		auto n = x * x + y * y + z * z + w * w;
		if (n == 1.f) return;

		n = std::sqrt(n);
		if (n < Math::F32_TOLERANCE) return;

		n = 1.f / n;

		x *= n;
		y *= n;
		z *= n;
		w *= n;
	}
}