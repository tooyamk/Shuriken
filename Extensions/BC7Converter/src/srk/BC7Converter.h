#pragma once

#include "srk/Image.h"

namespace srk::extensions {
	class SRK_EXTENSION_DLL BC7Converter {
	public:
		enum class Flags : uint8_t {
			NONE = 0,
			PERCEPTUAL = 1 << 0,

			WRITE_DDS_HEADER = 1 << 7
		};

		static bool SRK_CALL encode(const Image& img, uint32_t uberLevel, uint32_t maxPartitionsToScan, Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize);

		static ByteArray SRK_CALL encode(const Image& img, uint32_t uberLevel, uint32_t maxPartitionsToScan, Flags flags, size_t threadCount);
	};
}