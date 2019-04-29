#pragma once

#include "base/LowLevel.h"
#include "math/Size3.h"

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
			size(box.size) {
		}

		Box(Box<T>&& box) :
			left(box.left),
			top(box.top),
			front(box.front),
			size(box.size) {
		}

		Box(T left, T top, T front, const Size3<T>& size) :
			left(left),
			top(top),
			front(box.front),
			size(size) {
		}

		inline bool operator==(const Box<T> box) const {
			return left == box.left && top == box.top && front == box.front && size == rect.size;
		}

		inline bool operator!=(const Box<T> box) const {
			return left != box.left || top != box.top || front != box.front || size != rect.size;
		}

		inline void AE_CALL set(const Box<T>& box) {
			left = box.left;
			top = box.top;
			front = box.front;
			size.set(rect.size);
		}

		inline void AE_CALL set(T left, T top, T front, const Size3<T>& size) {
			this->left = left;
			this->top = top;
			this->front = front;
			this->size.set(size);
		}

		inline bool AE_CALL isEqual(const Box<T> box) const {
			return left == box.left && top == box.top && front == box.front && size == rect.size;
		}

		T left;
		T top;
		T front;
		Size3<T> size;
	};
}