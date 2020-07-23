#pragma once

#include "aurora/Global.h"
#include "aurora/math/Vector.h"

namespace aurora {
	template<uint32_t N, typename T>
	class AE_CORE_TMPL_DLL Box {
	public:
		template<typename K>
		using ConvertibleType = typename std::enable_if_t<std::is_convertible_v<K, T>, K>;

		Box() {
		}

		template<typename K, typename = ConvertibleType<K>>
		Box(const Box<N, K>& box) :
			pos(box.pos),
			size(box.size) {
		}

		template<typename K, typename = ConvertibleType<K>>
		Box(Box<N, K>&& box) :
			pos(box.pos),
			size(box.size) {
		}

		template<typename PT, typename ST, typename = ConvertibleType<PT>, typename = ConvertibleType<ST>>
		Box(const Vector<N, PT>&pos, const Vector<N, ST>& size) :
			pos(pos),
			size(size) {
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL operator==(const Box<N, K>& box) {
			return pos == box.pos && size == box.size;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL operator!=(const Box<N, K>& box) {
			return pos != box.pos || size != box.size;
		}

		template<typename K, typename = ConvertibleType<K>>
		inline void AE_CALL set(const Box<N, K>& box) {
			pos.set(box.pos);
			size.set(box.size);
		}

		template<typename PT, typename ST, typename = ConvertibleType<PT>, typename = ConvertibleType<ST>>
		inline void AE_CALL set(const Vector<N, PT>&pos, const Vector<N, ST>& size) {
			this->pos.set(pos);
			this->size.set(size);
		}

		template<typename K, typename = ConvertibleType<K>>
		inline bool AE_CALL isEqual(const Box<N, K>& box) const {
			return pos == box.pos && size == box.size;
		}

		Vector<N, T> pos;
		Vector<N, T> size;
	};

	template<typename T> using Box1 = Box<1, T>;
	using Box1f32 = Box1<float32_t>;
	using Box1i32 = Box1<int32_t>;
	using Box1ui32 = Box1<uint32_t>;
	template<typename T> using Box2 = Box<2, T>;
	using Box2f32 = Box2<float32_t>;
	using Box2i32 = Box2<int32_t>;
	using Box2ui32 = Box2<uint32_t>;
	template<typename T> using Box3 = Box<3, T>;
	using Box3f32 = Box3<float32_t>;
	using Box3i32 = Box3<int32_t>;
	using Box3ui32 = Box3<uint32_t>;
}