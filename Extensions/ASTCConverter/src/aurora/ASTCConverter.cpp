#include "ASTCConverter.h"
#include "ASTCConverterImpl.h"

namespace aurora::extensions {
	ByteArray ASTCConverter::encode(const Image& img, BlockSize blockSize, Preset preset, size_t threadCount) {
		return astc_converter::encode(img, blockSize, preset, threadCount);
	}
}