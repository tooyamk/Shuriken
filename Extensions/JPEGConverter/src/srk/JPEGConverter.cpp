#include "JPEGConverter.h"
#include "JPEGConverterImpl.h"

namespace srk::extensions {
	IntrusivePtr<Image> JPEGConverter::decode(const ByteArray& source) {
		return jpeg_converter::decode(source);
	}

	ByteArray JPEGConverter::encode(const Image& img, uint32_t quality) {
		return jpeg_converter::encode(img, quality);
	}
}