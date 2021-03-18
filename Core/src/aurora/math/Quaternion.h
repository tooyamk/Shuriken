#pragma once

#include "aurora/math/Math.h"

namespace aurora {
	class Matrix34;
	class Matrix44;

	class AE_CORE_DLL Quaternion {
	public:
		using Data = float32_t[4];

		Quaternion();
		Quaternion(const NoInit&);
		Quaternion(const Quaternion& v);
		Quaternion(Quaternion&& v) noexcept;
		Quaternion(float32_t x, float32_t y = 0.f, float32_t z = 0.f, float32_t w = 1.f);
		~Quaternion();

		static const Quaternion IDENTITY;

		inline AE_CALL operator Data& () {
			return data;
		}
		inline AE_CALL operator const Data& () const {
			return data;
		}

		inline float32_t AE_CALL getLength() const {
			return std::sqrt(getLengthSq());
		}
		inline float32_t AE_CALL getLengthSq() const {
			return x * x + y * y + z * z + w * w;
		}

		inline void AE_CALL set(const Quaternion& q) {
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
		}
		inline void AE_CALL set(float32_t x = 0.f, float32_t y = 0.f, float32_t z = 0.f, float32_t w = 1.f) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		inline void AE_CALL set(const float32_t(&q)[4]) {
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
		void AE_CALL toEuler(float32_t(&dst)[3]) const;
		inline bool AE_CALL isIdentity() const {
			return !memcmp(this, &IDENTITY, sizeof(Quaternion));
		}
		inline float32_t AE_CALL getRadian() const {
			return std::acos(w);
		}
		inline void AE_CALL mul(float32_t s) {
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
		inline void AE_CALL rotate(const float32_t(&p)[3], float32_t(&dst)[3]) const {
			Math::quatRotate<float32_t>(data, p, dst);
		}
		void AE_CALL toMatrix(Matrix34& dst) const;
		inline void AE_CALL toMatrix(Matrix44& dst) const {
			toMatrix((Matrix34&)dst);
		}

		static void AE_CALL createEulerX(float32_t radian, Quaternion& dst) {
			radian *= .5f;
			auto x = std::sin(radian);
			auto w = std::cos(radian);

			dst.set(x, 0.f, 0.f, w);
		}
		inline static Quaternion createEulerX(float32_t radian) {
			Quaternion q(NO_INIT);
			createEulerX(radian, q);
			return q;
		}
		inline static void AE_CALL createEulerY(float32_t radian, Quaternion& dst) {
			radian *= .5f;
			auto y = std::sin(radian);
			auto w = std::cos(radian);

			dst.set(0.f, y, 0.f, w);
		}
		inline static Quaternion AE_CALL createEulerY(float32_t radian) {
			Quaternion q(NO_INIT);
			createEulerY(radian, q);
			return q;
		}
		inline static void AE_CALL createEulerZ(float32_t radian, Quaternion& dst) {
			radian *= .5f;
			auto z = std::sin(radian);
			auto w = std::cos(radian);

			dst.set(0.f, 0.f, z, w);
		}
		inline static Quaternion AE_CALL createEulerZ(float32_t radian) {
			Quaternion q(NO_INIT);
			createEulerZ(radian, q);
			return q;
		}
		static void AE_CALL createEuler(const float32_t(&radians)[3], Quaternion& dst);
		inline static Quaternion AE_CALL createEuler(const float32_t(&radians)[3]) {
			Quaternion q(NO_INIT);
			createEuler(radians, q);
			return q;
		}
		static void AE_CALL createAxis(const float32_t(&axis)[3], float32_t radian, Quaternion& dst);
		static void AE_CALL createLookAt(const float32_t(&forward)[3], const float32_t(&upward)[3], Quaternion& dst);
		inline static void AE_CALL slerp(const Quaternion& from, const Quaternion& to, float32_t t, Quaternion& dst) {
			Math::slerp(from, to, t, dst);
		}
		inline static float32_t AE_CALL angleBetween(const Quaternion& q1, const Quaternion& q2) {
			return std::acos(Math::dot(q1.data, q2.data));
		}

		union {
			Data data;

			struct {
				float32_t x;
				float32_t y;
				float32_t z;
				float32_t w;
			};
		};
	};
}