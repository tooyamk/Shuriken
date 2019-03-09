#pragma once

#include "base/Aurora.h"

AE_NS_BEGIN

class AE_DLL Vector3 {
public:
	Vector3();
	Vector3(const Vector3& v);
	Vector3(Vector3&& v);
	Vector3(f32 x, f32 y = 0.f, f32 z = 0.f);
	~Vector3();

	static const Vector3 ZERO;
	static const Vector3 ONE;

	inline bool AE_CALL isZero() const;
	inline bool AE_CALL isOne() const;

	inline f32 AE_CALL getLength() const;
	inline f32 AE_CALL getLengthSq() const;

	inline void AE_CALL set(const Vector3& v);
	inline void AE_CALL set(f32 x = 0.f, f32 y = 0.f, f32 z = 0.f);
	inline void AE_CALL set(const f32* v);

	void AE_CALL setNormalize();

	inline static void AE_CALL normalize(const Vector3& v, Vector3& dst);
	inline static f32 AE_CALL dot(const Vector3& v1, const Vector3& v2);
	inline static void AE_CALL cross(const Vector3& v1, const Vector3& v2, Vector3& dst);
	static f32 AE_CALL angleBetween(const Vector3& v1, const Vector3& v2);
	inline f32 static AE_CALL distance(const Vector3& v1, const Vector3& v2);
	inline f32 static AE_CALL distanceSq(const Vector3& v1, const Vector3& v2);
	inline static void AE_CALL lerp(const Vector3& from, const Vector3& to, f32 t, Vector3& dst);

	union {
		f32 pos[3];

		struct {
			f32 x;
			f32 y;
			f32 z;
		};
	};
};

typedef Vector3 Vec3;

AE_NS_END

#include "Vector3.inl"