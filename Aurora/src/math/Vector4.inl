#include <xutility>
#include "Math.h"

namespace aurora {
	inline bool Vector4::isZero() const {
		return x == 0.f && y == 0.f && z == 0.f && w == 0.f;
	}

	inline bool Vector4::isOne() const {
		return x == 1.f && y == 1.f && z == 1.f && w == 1.f;
	}

	inline f32 Vector4::getLength() const {
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	inline f32 Vector4::getLengthSq() const {
		return x * x + y * y + z * z + w * w;
	}

	inline void Vector4::set(const Vector4& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	inline void Vector4::set(f32 x, f32 y, f32 z, f32 w) {
		r = x;
		g = y;
		b = z;
		a = w;
	}

	inline void Vector4::set(const f32* v) {
		x = v[0];
		y = v[1];
		z = v[2];
		w = v[3];
	}

	inline void Vector4::setRGBA(ui32 c) {
		r = (f32)(c >> 24 & 0xFF) / 255.f;
		g = (f32)(c >> 16 & 0xFF) / 255.f;
		b = (f32)(c >> 8 & 0xFF) / 255.f;
		a = (f32)(c & 0xFF) / 255.f;
	}

	inline void Vector4::normalize(const Vector4& v, Vector4& dst) {
		dst.set(v);
		dst.setNormalize();
	}

	inline f32 Vector4::dot(const Vector4& v1, const Vector4& v2) {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
	}

	inline f32 Vector4::distance(const Vector4& v1, const Vector4& v2) {
		auto x = v1.x - v2.x;
		auto y = v1.y - v2.y;
		auto z = v1.z - v2.z;
		auto w = v1.w - v2.w;

		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	inline f32 Vector4::distanceSq(const Vector4& v1, const Vector4& v2) {
		auto x = v1.x - v2.x;
		auto y = v1.y - v2.y;
		auto z = v1.z - v2.z;
		auto w = v1.w - v2.w;

		return x * x + y * y + z * z + w * w;
	}
}