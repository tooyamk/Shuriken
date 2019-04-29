#pragma once

#include "base/LowLevel.h"

namespace aurora {
	template<typename T>
	class AE_TEMPLATE_DLL Size3 {
	public:
		Size3() {
			memset(&width, 0, sizeof(T) * 3);
		}

		Size3(const Size3<T>& size) :
			width(size.width),
			height(size.height),
			depth(size.depth) {
		}

		Size3(Size3<T>&& size) :
			width(size.width),
			height(size.height),
			depth(size.depth) {
		}

		Size3(T width, T height, T depth) :
			width(width),
			height(height),
			depth(depth) {
		}

		inline bool operator==(const Size3<T>& size) const {
			return width == size.width && height == size.height && depth == size.depth;
		}

		inline bool operator!=(const Size3<T>& size) const {
			return width != size.width || height != size.height || depth != size.depth;
		}

		inline void AE_CALL set(const Size3<T>& size) {
			width = size.width;
			height = size.height;
			depth = size.depth;
		}

		inline void AE_CALL set(T width, T height, T depth) {
			this->width = width;
			this->height = height;
			this->depth = depth;
		}

		inline void AE_CALL set(T* values) {
			width = values[0];
			height = values[1];
			depth = values[2];
		}

		inline bool AE_CALL isEqual(const Size3<T> size) const {
			return width == size.width && height == size.height && depth == size.depth;
		}

		inline T AE_CALL getMax() const {
			T tmp = width > height ? width : height;
			return tmp > depth ? tmp : depth;
		}

		inline T AE_CALL getVolume() const {
			return width * height * depth;
		}

		T width;
		T height;
		T depth;
	};
}