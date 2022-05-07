#include "PNGConverter.h"
#include "PNGConverterImpl.h"

namespace srk::extensions {
	IntrusivePtr<Image> PNGConverter::decode(const ByteArray& source) {
		return png_converter::decode(source);
	}

	ByteArray PNGConverter::encode(const Image& img) {
		return png_converter::encode(img);
	}
}