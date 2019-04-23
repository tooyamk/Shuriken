#include "Image.h"

namespace aurora {
	Image::Image() :
		format(modules::graphics::TextureFormat::UNKNOWN),
		width(0),
		height(0) {
	}
}