#pragma once

#include "base/LowLevel.h"

namespace aurora {
	template<typename T>
	class AE_TEMPLATE_DLL Size2 {
	public:
		Size2() {
			memset(&width, 0, sizeof(T) << 1);
		}

		Size2(const Size2<T>& size) :
			width(size.width),
			height(size.height) {
		}

		Size2(Size2<T>&& size) :
			width(size.width),
			height(size.height) {
		}

		Size2(T width, T height) :
			width(width),
			height(height) {
		}

		inline void AE_CALL set(const Size2<T>& size) {
			width = size.width;
			height = size.height;
		}

		inline void AE_CALL set(T width, T height) {
			this->width = width;
			this->height = height;
		}

		inline void AE_CALL set(T* values) {
			width = values[0];
			height = values[1];
		}

		inline bool AE_CALL isEqual(const Size2<T> size) const {
			return width == size.width && height == size.height;
		}

		inline T AE_CALL getMax() const {
			return width > height ? width : height;
		}

		inline T AE_CALL getArea() const {
			return width * height;
		}

		T width;
		T height;
	};
}