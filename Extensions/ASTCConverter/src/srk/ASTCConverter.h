#pragma once

#include "srk/Image.h"

namespace srk::extensions {
	class SRK_EXTENSION_DLL ASTCConverter {
	public:
		/*enum class BlockSize : uint16_t {
			BLOCK_4x4,
			BLOCK_5x4,
			BLOCK_5x5,
			BLOCK_6x5,
			BLOCK_6x6,
			BLOCK_8x5,
			BLOCK_8x6,
			BLOCK_10x5,
			BLOCK_10x6,
			BLOCK_8x8,
			BLOCK_10x8,
			BLOCK_10x10,
			BLOCK_12x10,
			BLOCK_12x12,

			BLOCK_3x3x3,
			BLOCK_4x3x3,
			BLOCK_4x4x3,
			BLOCK_4x4x4,
			BLOCK_5x4x4,
			BLOCK_5x5x4,
			BLOCK_5x5x5,
			BLOCK_6x5x5,
			BLOCK_6x6x5,
			BLOCK_6x6x6
		};*/

		enum class Profile : uint8_t {
			LDR_SRGB = 0,
			LDR,
			HDR_RGB_LDR_A,
			HDR
		};

		enum class Preset : uint8_t {
			FASTEST,
			FAST,
			MEDIUM,
			THOROUGH,
			EXHAUSTIVE
		};

		enum class Flags : uint8_t {
			NONE = 0,
			MAP_NORMAL = 1 << 0,
			MAP_MASK = 1 << 1,
			USE_ALPHA_WEIGHT = 1 << 2,
			USE_PERCEPTUAL = 1 << 3,
			DECOMPRESS_ONLY = 1 << 4,
			SELF_DECOMPRESS_ONLY = 1 << 5,
			MAP_RGBM = 1 << 6,

			WRITE_HEADER = 1 << 7
		};

		static bool SRK_CALL encode(const Image& img, const Vector<3, uint8_t>& blockSize, Profile profile, Preset preset, Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize);

		static ByteArray SRK_CALL encode(const Image& img, const Vector<3, uint8_t>& blockSize, Profile profile, Preset preset, Flags flags, size_t threadCount);
	};
}