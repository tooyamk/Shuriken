#pragma once

#include "base/LowLevel.h"

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
			right(rect.right),
			bottom(rect.bottom) {
		}

		Rect(Rect<T>&& rect) :
			left(rect.left),
			top(rect.top),
			right(rect.right),
			bottom(rect.bottom) {
		}

		Rect(T left, T top, T right, T bottom) :
			left(left),
			top(top),
			right(right),
			bottom(bottom) {
		}

		inline T AE_CALL getWidth() const {
			return right - left;
		}

		inline T AE_CALL getHeight() const {
			return bottom - top;
		}

		inline void AE_CALL set(const Rect<T>& rect) {
			left = rect.left;
			right = rect.right;
			top = rect.top;
			bottom = rect.bottom;
		}

		inline void AE_CALL set(T left, T top, T right, T bottom) {
			this->left = left;
			this->top = top;
			this->right = right;
			this->bottom = bottom;
		}

		inline bool AE_CALL isEqual(const Rect<T> rect) const {
			return left == rect.left && right == rect.right && top == rect.top && bottom == rect.bottom;
		}

		T left;
		T top;
		T right;
		T bottom;
	};
}