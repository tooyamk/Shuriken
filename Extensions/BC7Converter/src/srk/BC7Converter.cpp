#include "BC7Converter.h"
#include "BC7ConverterImpl.h"

namespace srk::extensions {
	bool BC7Converter::encode(const Image& img, uint32_t uberLevel, uint32_t maxPartitionsToScan, Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize) {
		return bc7_converter::Impl::encode(img, uberLevel, maxPartitionsToScan, flags, threadCount, outBuffer, outBufferSize);
	}

	ByteArray BC7Converter::encode(const Image& img, uint32_t uberLevel, uint32_t maxPartitionsToScan, Flags flags, size_t threadCount) {
		void* buffer = nullptr;
		size_t bufferSize;
		if (!bc7_converter::Impl::encode(img, uberLevel, maxPartitionsToScan, flags, threadCount, &buffer, bufferSize)) return ByteArray();

		return ByteArray(buffer, bufferSize, ByteArray::Usage::EXCLUSIVE);
	}
}