#pragma once

#include "aurora/Image.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL ASTCConverter {
	public:
		AE_DECLARE_CANNOT_INSTANTIATE(ASTCConverter);

		enum class BlockSize : uint8_t {
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
		};

		enum class Preset : uint8_t {
			FASTEST,
			FAST,
			MEDIUM,
			THOROUGH,
			EXHAUSTIVE
		};

		static ByteArray AE_CALL encode(const Image& img, BlockSize blockSize, Preset preset, size_t threadCount = 0);
	};
}