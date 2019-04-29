#pragma once

#include "base/LowLevel.h"
#include "math/Size2.h"

namespace aurora {
	template<typename T>
	class AE_TEMPLATE_DLL Rect {
	public:
		Rect() {
			memset(&left, 0, sizeof(T) << 2);
		}

		Rect(const Rect<T>& rect) :
			left(rect.left),
			top(rect.top),
			size(rect.size) {
		}

		Rect(Rect<T>&& rect) :
			left(rect.left),
			top(rect.top),
			size(rect.size) {
		}

		Rect(T left, T top, const Size2<T>& size) :
			left(left),
			top(top),
			size(size) {
		}

		inline bool operator==(const Rect<T>& rect) const {
			return left == rect.left && top == rect.top && size == rect.size;
		}

		inline bool operator!=(const Rect<T>& rect) const {
			return left != rect.left || top != rect.top || size != rect.size;
		}

		inline void AE_CALL set(const Rect<T>& rect) {
			left = rect.left;
			top = rect.top;
			size.set(rect.size);
		}

		inline void AE_CALL set(T left, T top, const Size2<T>& size) {
			this->left = left;
			this->top = top;
			this->size.set(size);
		}

		inline bool AE_CALL isEqual(const Rect<T> rect) const {
			return left == rect.left && top == rect.top && size == rect.size;
		}

		T left;
		T top;
		Size2<T> size;
	};
}