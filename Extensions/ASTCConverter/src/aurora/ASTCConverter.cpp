#include "ASTCConverter.h"
#include "ASTCConverterImpl.h"

namespace aurora::extensions {
	ByteArray ASTCConverter::encode(const Image& img, BlockSize blockSize, Preset preset) {
		return astc_converter::encode(img, blockSize, preset);
	}
}