#include "JPEGConverter.h"
#include "JPEGConverterImpl.h"

namespace srk::extensions {
	IntrusivePtr<Image> JPEGConverter::decode(const ByteArray& source) {
		return jpeg_converter::decode(source);
	}

	/*ByteArray JPEGConverter::encode(const Image& img) {
		return png_converter::encode(img);
	}*/
}