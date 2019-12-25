#pragma once

#include "base/Global.h"
#include "math/Vector.h"

namespace aurora {
	template<uint32_t N, typename T>
	class AE_TEMPLATE_DLL Box {
	public:
		Box() {
		}

		Box(const Box& box) :
			pos(box.pos),
			size(box.size) {
		}

		Box(Box&& box) :
			pos(box.pos),
			size(box.size) {
		}

		Box(const Vec<N, T>&pos, const Vec<N, T>& size) :
			pos(pos),
			size(size) {
		}

		inline bool AE_CALL operator==(const Box& box) {
			return pos == box.pos && size == box.size;
		}

		inline bool AE_CALL operator!=(const Box& box) {
			return pos != box.pos || size != box.size;
		}

		inline void AE_CALL set(const Box& box) {
			pos.set(box.pos);
			size.set(box.size);
		}

		inline void AE_CALL set(const Vec<N, T>&pos, const Vec<N, T>& size) {
			this->pos.set(pos);
			this->size.set(size);
		}

		inline bool AE_CALL isEqual(const Box& box) const {
			return pos == box.pos && size == box.size;
		}

		Vec<N, T> pos;
		Vec<N, T> size;
	};

	template<typename T> using Box1 = Box<1, T>;
	using Box1f32 = Box1<f32>;
	using Box1i32 = Box1<int32_t>;
	using Box1ui32 = Box1<uint32_t>;
	template<typename T> using Box2 = Box<2, T>;
	using Box2f32 = Box2<f32>;
	using Box2i32 = Box2<int32_t>;
	using Box2ui32 = Box2<uint32_t>;
	template<typename T> using Box3 = Box<3, T>;
	using Box3f32 = Box3<f32>;
	using Box3i32 = Box3<int32_t>;
	using Box3ui32 = Box3<uint32_t>;
}