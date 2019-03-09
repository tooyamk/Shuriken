#pragma once

#include "base/Aurora.h"

AE_NS_BEGIN

class AE_DLL Vector4 {
public:
	Vector4();
	Vector4(const Vector4& v);
	Vector4(Vector4&& v);
	Vector4(f32 x, f32 y = 0.f, f32 z = 0.f, f32 w = 0.f);
	~Vector4();

	static const Vector4 ZERO;
	static const Vector4 ONE;

	inline bool AE_CALL isZero() const;
	inline bool AE_CALL isOne() const;

	inline f32 AE_CALL getLength() const;
	inline f32 AE_CALL getLengthSq() const;

	inline void AE_CALL set(const Vector4& v);
	inline void AE_CALL set(f32 x = 0.f, f32 y = 0.f, f32 z = 0.f, f32 w = 0.f);
	inline void AE_CALL set(const f32* v);
	inline void AE_CALL setRGBA(ui32 c);

	void AE_CALL setNormalize();

	inline static void AE_CALL normalize(const Vector4& v, Vector4& dst);
	inline static f32 AE_CALL dot(const Vector4& v1, const Vector4& v2);
	inline f32 static AE_CALL distance(const Vector4& v1, const Vector4& v2);
	inline f32 static AE_CALL distanceSq(const Vector4& v1, const Vector4& v2);

	union {
		f32 pos[4];

		struct {
			f32 x;
			f32 y;
			f32 z;
			f32 w;
		};

		struct {
			f32 r;
			f32 g;
			f32 b;
			f32 a;
		};
	};
};

typedef Vector4 Vec4;

AE_NS_END

#include "Vector4.inl"