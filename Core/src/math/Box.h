#pragma once

#include "base/LowLevel.h"
#include "math/Vector.h"

namespace aurora {
	template<ui32 N, typename T>
	class AE_TEMPLATE_DLL Box {
	public:
		Box() {
		}

		Box(const Box<N, T>& box) :
			pos(box.pos),
			size(box.size) {
		}

		Box(Box<N, T>&& box) :
			pos(box.pos),
			size(box.size) {
		}

		Box(const Vec<N, T>&pos, const Vec<N, T>& size) :
			pos(pos),
			size(size) {
		}

		inline bool AE_CALL operator==(const Box<N, T>& box) {
			return pos == box.pos && size == box.size;
		}

		inline bool AE_CALL operator!=(const Box<N, T>& box) {
			return pos != box.pos || size != box.size;
		}

		inline void AE_CALL set(const Box<N, T>& box) {
			pos.set(box.pos);
			size.set(box.size);
		}

		inline void AE_CALL set(const Vec<N, T>&pos, const Vec<N, T>& size) {
			this->pos.set(pos);
			this->size.set(size);
		}

		inline bool AE_CALL isEqual(const Box<N, T>& box) const {
			return pos == box.pos && size == box.size;
		}

		Vec<N, T> pos;
		Vec<N, T> size;
	};

	template<typename T> using Box1 = Box<1, T>;
	using Box1f32 = Box1<f32>;
	using Box1i32 = Box1<i32>;
	using Box1ui32 = Box1<ui32>;
	template<typename T> using Box2 = Box<2, T>;
	using Box2f32 = Box2<f32>;
	using Box2i32 = Box2<i32>;
	using Box2ui32 = Box2<ui32>;
	template<typename T> using Box3 = Box<3, T>;
	using Box3f32 = Box3<f32>;
	using Box3i32 = Box3<i32>;
	using Box3ui32 = Box3<ui32>;
}