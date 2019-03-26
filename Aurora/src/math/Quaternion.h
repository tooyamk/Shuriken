#pragma once

#include "base/LowLevel.h"

namespace aurora {
	class Matrix34;
	class Matrix44;
	class Vector3;

	class AE_DLL Quaternion {
	public:
		Quaternion();
		Quaternion(const Quaternion& v);
		Quaternion(Quaternion&& v);
		Quaternion(f32 x, f32 y = 0.f, f32 z = 0.f, f32 w = 1.f);
		~Quaternion();

		static const Quaternion IDENTITY;

		inline f32 AE_CALL length() const;
		inline f32 AE_CALL lengthSq() const;

		inline void AE_CALL set(const Quaternion& q);
		inline void AE_CALL set(f32 x = 0.f, f32 y = 0.f, f32 z = 0.f, f32 w = 1.f);
		inline void AE_CALL set(const f32* q);

		void AE_CALL normalize();
		inline void AE_CALL conjugate();
		inline void AE_CALL invert();
		inline void AE_CALL invert(Quaternion& dst) const;
		void AE_CALL toEuler(Vector3& dst) const;
		inline bool AE_CALL isIdentity() const;
		inline f32 AE_CALL getRadian() const;
		inline void AE_CALL mul(f32 s);
		inline void AE_CALL append(const Quaternion& q);
		inline void AE_CALL append(const Quaternion& q, Quaternion& dst) const;
		inline void AE_CALL rotate(const Vector3& p, Vector3& dst) const;
		inline void AE_CALL toMatrix(Matrix34& dst) const;
		inline void AE_CALL toMatrix(Matrix44& dst) const;

		static void AE_CALL createFromEulerX(f32 radian, Quaternion& dst);
		static void AE_CALL createFromEulerY(f32 radian, Quaternion& dst);
		static void AE_CALL createFromEulerZ(f32 radian, Quaternion& dst);
		static void AE_CALL createFromEuler(const Vector3& radians, Quaternion& dst);
		static void AE_CALL createFromAxis(const Vector3& axis, f32 radian, Quaternion& dst);
		static void AE_CALL createLookAt(const Vector3& forward, const Vector3& upward, Quaternion& dst);
		inline static void AE_CALL slerp(const Quaternion& from, const Quaternion& to, f32 t, Quaternion& dst);
		inline static f32 AE_CALL dot(const Quaternion& q1, const Quaternion& q2);
		inline static f32 AE_CALL angleBetween(const Quaternion& q1, const Quaternion& q2);

		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
}

#include "Quaternion.inl"