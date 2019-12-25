#include "Quaternion.h"
#include "math/Matrix34.h"

namespace aurora {
	inline Quaternion::operator Quaternion::Data& () {
 		return data;
 	}

	inline Quaternion::operator const Quaternion::Data& () const {
		return data;
	}

	inline f32 Quaternion::length() const {
		return std::sqrt(lengthSq());
	}

	inline f32 Quaternion::lengthSq() const {
		return x * x + y * y + z * z + w * w;
	}

	inline void Quaternion::set(const Quaternion& q) {
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
	}

	inline void Quaternion::set(f32 x, f32 y, f32 z, f32 w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	inline void Quaternion::set(const f32(&q)[4]) {
		x = q[0];
		y = q[1];
		z = q[2];
		w = q[3];
	}

	inline void Quaternion::conjugate() {
		x = -x;
		y = -y;
		z = -z;
	}

	inline void Quaternion::invert() {
		x = -x;
		y = -y;
		z = -z;
	}

	inline void Quaternion::invert(Quaternion& dst) const {
		dst.x = -x;
		dst.y = -y;
		dst.z = -z;
		dst.w = w;
	}

	inline bool Quaternion::isIdentity() const {
		return x == 0.f && y == 0.f && z == 0.f && w == 1.f;
	}

	inline f32 Quaternion::getRadian() const {
		return std::acos(w);
	}

	inline void Quaternion::mul(f32 s) {
		x *= s;
		y *= s;
		z *= s;
		w *= s;
	}

	inline void Quaternion::append(const Quaternion& q) {
		append(q, *this);
	}

	inline void Quaternion::append(const Quaternion& q, Quaternion& dst) const {
		Math::appendQuat(*this, q, dst);
	}

	inline void Quaternion::rotate(const f32(&p)[3], f32(&dst)[3]) const {
		Math::quatRotate<f32>(data, p, dst);
	}

	inline void Quaternion::toMatrix(Matrix34& dst) const {
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

	inline void Quaternion::toMatrix(Matrix44& dst) const {
		toMatrix((Matrix34&)dst);
	}

	inline void Quaternion::slerp(const Quaternion& from, const Quaternion& to, f32 t, Quaternion& dst) {
		Math::slerpQuat(&from.x, &to.x, t, &dst.x);
	}

	inline f32 Quaternion::dot(const Quaternion& q1, const Quaternion& q2) {
		return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	}

	inline f32 Quaternion::angleBetween(const Quaternion& q1, const Quaternion& q2) {
		return std::acos(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
	}
}