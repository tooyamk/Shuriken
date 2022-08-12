#pragma once

#include "srk/math/Math.h"

namespace srk {
	class Matrix34;
	class Matrix44;

	class SRK_CORE_DLL Quaternion {
	public:
		using Data = float32_t[4];

		Quaternion();
		Quaternion(const NoInit&);
		Quaternion(const Quaternion& v);
		Quaternion(Quaternion&& v) noexcept;
		Quaternion(float32_t x, float32_t y = 0.f, float32_t z = 0.f, float32_t w = 1.f);
		~Quaternion();

		static const Quaternion IDENTITY;

		inline SRK_CALL operator Data& () {
			return data;
		}
		inline SRK_CALL operator const Data& () const {
			return data;
		}

		inline Quaternion& SRK_CALL operator=(const Quaternion& q) noexcept {
			for (auto i = 0; i < 4; ++i) data[i] = q.data[i];
			return *this;
		}

		inline Quaternion& SRK_CALL operator=(Quaternion&& q) noexcept {
			for (auto i = 0; i < 4; ++i) data[i] = q.data[i];
			return *this;
		}

		inline void SRK_CALL operator*=(const Quaternion& q) {
			append(q);
		}

		inline float32_t SRK_CALL getLength() const {
			return std::sqrt(getLengthSq());
		}
		inline float32_t SRK_CALL getLengthSq() const {
			return x * x + y * y + z * z + w * w;
		}

		inline void SRK_CALL set(const Quaternion& q) {
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
		}
		inline void SRK_CALL set(float32_t x = 0.f, float32_t y = 0.f, float32_t z = 0.f, float32_t w = 1.f) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		inline void SRK_CALL set(const float32_t(&q)[4]) {
			x = q[0];
			y = q[1];
			z = q[2];
			w = q[3];
		}

		void SRK_CALL normalize();
		inline void SRK_CALL conjugate() {
			x = -x;
			y = -y;
			z = -z;
		}
		inline void SRK_CALL invert() {
			x = -x;
			y = -y;
			z = -z;
		}
		inline void SRK_CALL invert(Quaternion& dst) const {
			dst.x = -x;
			dst.y = -y;
			dst.z = -z;
			dst.w = w;
		}
		void SRK_CALL toEuler(float32_t(&dst)[3]) const;
		inline bool SRK_CALL isIdentity() const {
			return !memcmp(this, &IDENTITY, sizeof(Quaternion));
		}
		inline float32_t SRK_CALL getRadian() const {
			return std::acos(w);
		}
		inline void SRK_CALL mul(float32_t s) {
			x *= s;
			y *= s;
			z *= s;
			w *= s;
		}
		inline void SRK_CALL append(const Quaternion& q) {
			append(q, *this);
		}
		inline void SRK_CALL append(const Quaternion& q, Quaternion& dst) const {
			Math::appendQuat(data, q.data, dst.data);
		}
		inline void SRK_CALL rotate(const float32_t(&p)[3], float32_t(&dst)[3]) const {
			Math::rotateQuat<float32_t>(data, p, dst);
		}
		void SRK_CALL toMatrix(Matrix34& dst) const;
		inline void SRK_CALL toMatrix(Matrix44& dst) const {
			toMatrix((Matrix34&)dst);
		}

		static void SRK_CALL createEulerX(float32_t radian, Quaternion& dst) {
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
		inline static void SRK_CALL createEulerY(float32_t radian, Quaternion& dst) {
			radian *= .5f;
			auto y = std::sin(radian);
			auto w = std::cos(radian);

			dst.set(0.f, y, 0.f, w);
		}
		inline static Quaternion SRK_CALL createEulerY(float32_t radian) {
			Quaternion q(NO_INIT);
			createEulerY(radian, q);
			return q;
		}
		inline static void SRK_CALL createEulerZ(float32_t radian, Quaternion& dst) {
			radian *= .5f;
			auto z = std::sin(radian);
			auto w = std::cos(radian);

			dst.set(0.f, 0.f, z, w);
		}
		inline static Quaternion SRK_CALL createEulerZ(float32_t radian) {
			Quaternion q(NO_INIT);
			createEulerZ(radian, q);
			return q;
		}
		static void SRK_CALL createEuler(const float32_t(&radians)[3], Quaternion& dst);
		inline static Quaternion SRK_CALL createEuler(const float32_t(&radians)[3]) {
			Quaternion q(NO_INIT);
			createEuler(radians, q);
			return q;
		}
		static void SRK_CALL createAxis(const float32_t(&axis)[3], float32_t radian, Quaternion& dst);
		inline static Quaternion SRK_CALL createAxis(const float32_t(&axis)[3], float32_t radian) {
			Quaternion q(NO_INIT);
			createAxis(axis, radian, q);
			return q;
		}
		static void SRK_CALL createLookAt(const float32_t(&forward)[3], const float32_t(&upward)[3], Quaternion& dst);
		inline static void SRK_CALL slerp(const Quaternion& from, const Quaternion& to, float32_t t, Quaternion& dst) {
			Math::slerp(from.data, to.data, t, dst.data);
		}
		inline static float32_t SRK_CALL angleBetween(const Quaternion& q1, const Quaternion& q2) {
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

	inline Quaternion SRK_CALL operator*(const Quaternion& lhs, const Quaternion& rhs) {
		Quaternion q(NO_INIT);
		Math::appendQuat(lhs.data, rhs.data, q.data);
		return q;
	}
}