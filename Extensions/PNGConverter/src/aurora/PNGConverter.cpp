#include "PNGConverter.h"
#include "PNGConverterImpl.h"

namespace aurora::extensions {
	Image* PNGConverter::parse(const ByteArray& source) {
		return png_converter::parse(source);
	}

	ByteArray PNGConverter::encode(const Image& img) {
		return png_converter::encode(img);
	}
}