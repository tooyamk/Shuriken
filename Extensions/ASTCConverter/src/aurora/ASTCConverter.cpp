#include "ASTCConverter.h"
#include "ASTCConverterImpl.h"

namespace aurora::extensions {
	ByteArray ASTCConverter::encode(const Image& img, Vec3<uint8_t> blockSize, ASTCConverter::Preset preset, ASTCConverter::Flags flags, size_t threadCount) {
		return astc_converter::encode(img, blockSize, preset, flags, threadCount);
	}
}