#include <xutility>
#include "Math.h"

namespace aurora {
	inline bool Vector2::isZero() const {
		return x == 0.f && y == 0.f;
	}

	inline bool Vector2::isOne() const {
		return x == 1.f && y == 1.f;
	}

	inline f32 Vector2::getLength() const {
		return std::sqrt(x * x + y * y);
	}

	inline f32 Vector2::getLengthSq() const {
		return x * x + y * y;
	}

	inline void Vector2::set(const Vector2& v) {
		x = v.x;
		y = v.y;
	}

	inline void Vector2::set(f32 x, f32 y) {
		this->x = x;
		this->y = y;
	}

	inline void Vector2::set(const f32* v) {
		x = v[0];
		y = v[1];
	}

	inline void Vector2::normalize(const Vector2& v, Vector2& dst) {
		dst.set(v);
		dst.setNormalize();
	}

	inline f32 Vector2::dot(const Vector2& v1, const Vector2& v2) {
		return v1.x * v2.x + v1.y * v2.y;
	}

	inline f32 Vector2::distance(const Vector2& v1, const Vector2& v2) {
		auto x = v1.x - v2.x;
		auto y = v1.y - v2.y;

		return std::sqrt(x * x + y * y);
	}

	inline f32 Vector2::distanceSq(const Vector2& v1, const Vector2& v2) {
		auto x = v1.x - v2.x;
		auto y = v1.y - v2.y;

		return x * x + y * y;
	}

	inline void Vector2::lerp(const Vector2& from, const Vector2& to, f32 t, Vector2& dst) {
		Math::lerpVec(from.pos, to.pos, t, dst.pos);
	}
}