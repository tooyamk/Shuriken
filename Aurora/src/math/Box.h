#pragma once

#include "base/LowLevel.h"

namespace aurora {
	template<typename T>
	class AE_TEMPLATE_DLL Box {
	public:
		Box() {
			memset(&left, 0, sizeof(T) * 6);
		}

		Box(const Box<T>& box) :
			left(box.left),
			top(box.top),
			front(box.front),
			right(box.right),
			bottom(box.bottom),
			back(box.back) {
		}

		Box(Box<T>&& box) :
			left(box.left),
			top(box.top),
			front(box.front),
			right(box.right),
			bottom(box.bottom),
			back(box.back) {
		}

		Box(T left, T top, T front, T right, T bottom, T back) :
			left(left),
			top(top),
			front(box.front),
			right(right),
			bottom(bottom),
			back(box.back) {
		}

		inline T AE_CALL getWidth() const {
			return right - left;
		}

		inline T AE_CALL getHeight() const {
			return bottom - top;
		}

		inline void AE_CALL set(const Box<T>& box) {
			left = box.left;
			right = box.right;
			front = box.front;
			top = box.top;
			bottom = box.bottom;
			back = box.back;
		}

		inline void AE_CALL set(T left, T top, T front, T right, T bottom, T back) {
			this->left = left;
			this->top = top;
			this->front = front;
			this->right = right;
			this->bottom = bottom;
			this->back = back;
		}

		inline bool AE_CALL isEqual(const Box<T> box) const {
			return left == box.left && right == box.right && top == box.top && bottom == box.bottom && front == box.front && back = box.back;
		}

		T left;
		T top;
		T front;
		T right;
		T bottom;
		T back;
	};
}