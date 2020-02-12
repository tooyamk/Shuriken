#pragma once

#include "aurora/math/Math.h"

namespace aurora {
	class Matrix34;
	class Matrix44;

	class AE_DLL Quaternion {
	public:
		using Data = f32[4];

		Quaternion();
		Quaternion(const Quaternion& v);
		Quaternion(Quaternion&& v) noexcept;
		Quaternion(f32 x, f32 y = 0.f, f32 z = 0.f, f32 w = 1.f);
		~Quaternion();

		static const Quaternion IDENTITY;

		inline AE_CALL operator Data& () {
			return data;
		}
		inline AE_CALL operator const Data& () const {
			return data;
		}

		inline f32 AE_CALL getLength() const {
			return std::sqrt(getLengthSq());
		}
		inline f32 AE_CALL getLengthSq() const {
			return x * x + y * y + z * z + w * w;
		}

		inline void AE_CALL set(const Quaternion& q) {
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
		}
		inline void AE_CALL set(f32 x = 0.f, f32 y = 0.f, f32 z = 0.f, f32 w = 1.f) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		inline void AE_CALL set(const f32(&q)[4]) {
			x = q[0];
			y = q[1];
			z = q[2];
			w = q[3];
		}

		void AE_CALL normalize();
		inline void AE_CALL conjugate() {
			x = -x;
			y = -y;
			z = -z;
		}
		inline void AE_CALL invert() {
			x = -x;
			y = -y;
			z = -z;
		}
		inline void AE_CALL invert(Quaternion& dst) const {
			dst.x = -x;
			dst.y = -y;
			dst.z = -z;
			dst.w = w;
		}
		void AE_CALL toEuler(f32(&dst)[3]) const;
		inline bool AE_CALL isIdentity() const {
			return x == 0.f && y == 0.f && z == 0.f && w == 1.f;
		}
		inline f32 AE_CALL getRadian() const {
			return std::acos(w);
		}
		inline void AE_CALL mul(f32 s) {
			x *= s;
			y *= s;
			z *= s;
			w *= s;
		}
		inline void AE_CALL append(const Quaternion& q) {
			append(q, *this);
		}
		inline void AE_CALL append(const Quaternion& q, Quaternion& dst) const {
			Math::appendQuat(*this, q, dst);
		}
		inline void AE_CALL rotate(const f32(&p)[3], f32(&dst)[3]) const {
			Math::quatRotate<f32>(data, p, dst);
		}
		void AE_CALL toMatrix(Matrix34& dst) const;
		inline void AE_CALL toMatrix(Matrix44& dst) const {
			toMatrix((Matrix34&)dst);
		}

		static void AE_CALL createFromEulerX(f32 radian, Quaternion& dst);
		static void AE_CALL createFromEulerY(f32 radian, Quaternion& dst);
		static void AE_CALL createFromEulerZ(f32 radian, Quaternion& dst);
		static void AE_CALL createFromEuler(const f32(&radians)[3], Quaternion& dst);
		static void AE_CALL createFromAxis(const f32(&axis)[3], f32 radian, Quaternion& dst);
		static void AE_CALL createLookAt(const f32(&forward)[3], const f32(&upward)[3], Quaternion& dst);
		inline static void AE_CALL slerp(const Quaternion& from, const Quaternion& to, f32 t, Quaternion& dst) {
			Math::slerpQuat(&from.x, &to.x, t, &dst.x);
		}
		inline static f32 AE_CALL dot(const Quaternion& q1, const Quaternion& q2) {
			return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
		}
		inline static f32 AE_CALL angleBetween(const Quaternion& q1, const Quaternion& q2) {
			return std::acos(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
		}

		union {
			Data data;

			struct {
				f32 x;
				f32 y;
				f32 z;
				f32 w;
			};
		};
	};
}