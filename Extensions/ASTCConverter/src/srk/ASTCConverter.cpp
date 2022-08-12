#include "ASTCConverter.h"
#include "ASTCConverterImpl.h"

namespace srk::extensions {
	bool ASTCConverter::encode(const Image& img, const Vec3<uint8_t>& blockSize, Profile profile, Quality quality, Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize) {
		return astc_converter::Impl::encode(img, blockSize, profile, quality, flags, threadCount, outBuffer, outBufferSize);
	}

	ByteArray ASTCConverter::encode(const Image& img, const Vector<3, uint8_t>& blockSize, Profile profile, Quality quality, Flags flags, size_t threadCount) {
		void* buffer = nullptr;
		size_t bufferSize;
		if (!astc_converter::Impl::encode(img, blockSize, profile, quality, flags, threadCount, &buffer, bufferSize)) return ByteArray();

		return ByteArray(buffer, bufferSize, ByteArray::Usage::EXCLUSIVE);
	}
}