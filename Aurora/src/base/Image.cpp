#include "Image.h"

namespace aurora {
	Image::Image() :
		width(0),
		height(0) {
	}

	Image::Image(Image&& img) :
		width(img.width),
		height(img.height),
		source(std::move(img.source)) {
	}

	Image& Image::operator=(Image&& img) {
		width = img.width;
		height = img.height;
		source = std::move(img.source);

		return *this;
	}
}