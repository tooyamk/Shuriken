#pragma once

#include "srk/math/Vector.h"

namespace srk {
	template<size_t N, typename Pos, typename Size>
	class Box {
	public:
		Box() {
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		Box(const Box<N, P, S>& box) :
			pos(box.pos),
			size(box.size) {
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		Box(Box<N, P, S>&& box) :
			pos(box.pos),
			size(box.size) {
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		Box(const Vector<N, P>&pos, const Vector<N, S>& size) :
			pos(pos),
			size(size) {
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		inline bool SRK_CALL operator==(const Box<N, P, S>& box) {
			return pos == box.pos && size == box.size;
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		inline bool SRK_CALL operator!=(const Box<N, P, S>& box) {
			return pos != box.pos || size != box.size;
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		inline void SRK_CALL set(const Box<N, P, S>& box) {
			pos.set(box.pos);
			size.set(box.size);
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		inline void SRK_CALL set(const Vector<N, P>&pos, const Vector<N, S>& size) {
			this->pos.set(pos);
			this->size.set(size);
		}

		template<std::convertible_to<Pos> P, std::convertible_to<Size> S>
		inline bool SRK_CALL equal(const Box<N, P, S>& box) const {
			return pos == box.pos && size == box.size;
		}

		Vector<N, Pos> pos;
		Vector<N, Size> size;
	};

	template<typename P, typename S> using Box1 = Box<1, P, S>;
	using Box1f32 = Box1<float32_t, float32_t>;
	using Box1i32ui32 = Box1<int32_t, uint32_t>;
	using Box1i32 = Box1<int32_t, int32_t>;
	using Box1ui32 = Box1<uint32_t, uint32_t>;
	using Box1uz = Box1<size_t, size_t>;
	template<typename P, typename S> using Box2 = Box<2, P, S>;
	using Box2f32 = Box2<float32_t, float32_t>;
	using Box2i32ui32 = Box2<int32_t, uint32_t>;
	using Box2i32 = Box2<int32_t, int32_t>;
	using Box2ui32 = Box2<uint32_t, uint32_t>;
	using Box2uz = Box2<size_t, size_t>;
	template<typename P, typename S> using Box3 = Box<3, P, S>;
	using Box3f32 = Box3<float32_t, float32_t>;
	using Box3i32ui32 = Box3<int32_t, uint32_t>;
	using Box3i32 = Box3<int32_t, int32_t>;
	using Box3ui32 = Box3<uint32_t, uint32_t>;
	using Box3uz = Box3<size_t, size_t>;
}