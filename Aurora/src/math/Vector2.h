#pragma once

#include "base/LowLevel.h"

namespace aurora {
	class AE_DLL Vector2 {
	public:
		Vector2();
		Vector2(const Vector2& v);
		Vector2(Vector2&& v);
		Vector2(f32 x, f32 y = 0.f);
		~Vector2();

		static const Vector2 ZERO;
		static const Vector2 ONE;

		inline bool AE_CALL isZero() const;
		inline bool AE_CALL isOne() const;

		inline f32 AE_CALL getLength() const;
		inline f32 AE_CALL getLengthSq() const;

		inline void AE_CALL set(const Vector2& v);
		inline void AE_CALL set(f32 x = 0.f, f32 y = 0.f);
		inline void AE_CALL set(const f32* v);

		void AE_CALL setNormalize();

		inline static void AE_CALL normalize(const Vector2& v, Vector2& dst);
		inline static f32 AE_CALL dot(const Vector2& v1, const Vector2& v2);
		static f32 AE_CALL angleBetween(const Vector2& v1, const Vector2& v2);
		inline f32 static AE_CALL distance(const Vector2& v1, const Vector2& v2);
		inline f32 static AE_CALL distanceSq(const Vector2& v1, const Vector2& v2);
		inline static void AE_CALL lerp(const Vector2& from, const Vector2& to, f32 t, Vector2& dst);

		union {
			f32 pos[2];

			struct {
				f32 x;
				f32 y;
			};
		};
	};

	using Vec2 = Vector2;
}

#include "Vector2.inl"