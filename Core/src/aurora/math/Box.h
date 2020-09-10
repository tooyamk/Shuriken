#pragma once

#include "aurora/Global.h"
#include "aurora/math/Vector.h"

namespace aurora {
	template<uint32_t N, typename Pos, typename Size>
	class Box {
	public:
		template<typename S, typename T>
		using ConvertibleType = typename std::enable_if_t<std::is_convertible_v<S, T>, S>;

		template<typename S1, typename T1, typename S2, typename T2>
		using ConvertibleTypes = typename std::enable_if_t<std::is_convertible_v<S1, T1> && std::is_convertible_v<S2, T2>, bool>;

		Box() {
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		Box(const Box<N, P, S>& box) :
			pos(box.pos),
			size(box.size) {
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		Box(Box<N, P, S>&& box) :
			pos(box.pos),
			size(box.size) {
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		Box(const Vector<N, P>&pos, const Vector<N, S>& size) :
			pos(pos),
			size(size) {
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		inline bool AE_CALL operator==(const Box<N, P, S>& box) {
			return pos == box.pos && size == box.size;
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		inline bool AE_CALL operator!=(const Box<N, P, S>& box) {
			return pos != box.pos || size != box.size;
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		inline void AE_CALL set(const Box<N, P, S>& box) {
			pos.set(box.pos);
			size.set(box.size);
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		inline void AE_CALL set(const Vector<N, P>&pos, const Vector<N, S>& size) {
			this->pos.set(pos);
			this->size.set(size);
		}

		template<typename P, typename S, typename = ConvertibleTypes<P, Pos, S, Size>>
		inline bool AE_CALL isEqual(const Box<N, P, S>& box) const {
			return pos == box.pos && size == box.size;
		}

		Vector<N, Pos> pos;
		Vector<N, Size> size;
	};

	template<typename P, typename S> using Box1 = Box<1, P, S>;
	using Box1f32 = Box1<float32_t, float32_t>;
	using Box1i32ui32 = Box1<int32_t, uint32_t>;
	using Box1ui32 = Box1<uint32_t, uint32_t>;
	template<typename P, typename S> using Box2 = Box<2, P, S>;
	using Box2f32 = Box2<float32_t, float32_t>;
	using Box2i32ui32 = Box2<int32_t, uint32_t>;
	using Box2ui32 = Box2<uint32_t, uint32_t>;
	template<typename P, typename S> using Box3 = Box<3, P, S>;
	using Box3f32 = Box3<float32_t, float32_t>;
	using Box3i32ui32 = Box3<int32_t, uint32_t>;
	using Box3ui32 = Box3<uint32_t, uint32_t>;
}