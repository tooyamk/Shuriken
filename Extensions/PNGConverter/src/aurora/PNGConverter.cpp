#include "PNGConverter.h"
#include "PNGConverterImpl.h"

namespace aurora::extensions {
	Image* PNGConverter::parse(const ByteArray& source) {
		return png_converter::parse(source);
	}
}