#include <xutility>
#include "Math.h"

AE_NS_BEGIN

inline bool Vector3::isZero() const {
	return x == 0.f && y == 0.f && z == 0.f;
}

inline bool Vector3::isOne() const {
	return x == 1.f && y == 1.f && z == 1.f;
}

inline f32 Vector3::getLength() const {
	return std::sqrt(x * x + y * y + z * z);
}

inline f32 Vector3::getLengthSq() const {
	return x * x + y * y + z * z;
}

inline void Vector3::set(const Vector3& v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

inline void Vector3::set(f32 x, f32 y, f32 z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

inline void Vector3::set(const f32* v) {
	x = v[0];
	y = v[1];
	z = v[2];
}

inline void Vector3::normalize(const Vector3& v, Vector3& dst) {
	dst.set(v);
	dst.setNormalize();
}

inline f32 Vector3::dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline void Vector3::cross(const Vector3& v1, const Vector3& v2, Vector3& dst) {
	Math::crossVec3(&v1.x, &v2.x, &dst.x);
}

inline f32 Vector3::distance(const Vector3& v1, const Vector3& v2) {
	auto x = v1.x - v2.x;
	auto y = v1.y - v2.y;
	auto z = v1.z - v2.z;

	return std::sqrt(x * x + y * y + z * z);
}

inline f32 Vector3::distanceSq(const Vector3& v1, const Vector3& v2) {
	auto x = v1.x - v2.x;
	auto y = v1.y - v2.y;
	auto z = v1.z - v2.z;

	return x * x + y * y + z * z;
}

inline void Vector3::lerp(const Vector3& from, const Vector3& to, f32 t, Vector3& dst) {
	Math::lerpVec3(&from.x, &to.x, t, &dst.x);
}

AE_NS_END