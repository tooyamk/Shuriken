#pragma once

#include "srk/Image.h"
#include <functional>
#include <future>

#ifdef SRK_EXT_ASTC_CONV_EXPORTS
#	define SRK_EXT_ASTC_CONV_DLL SRK_DLL_EXPORT
#else
#	define SRK_EXT_ASTC_CONV_DLL SRK_DLL_IMPORT
#endif

namespace srk::extensions {
	class SRK_EXT_ASTC_CONV_DLL ASTCConverter {
	public:
		static constexpr size_t HEADER_SIZE = 16;
		static constexpr uint32_t HEADER_MAGIC_ID = 0x5CA1AB13;

		using Job = std::function<std::shared_future<void>(const std::function<void()>&)>;

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

		enum class Quality : uint8_t {
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

		static bool SRK_CALL encode(const Image& img, const Vector<3, uint8_t>& blockSize, Profile profile, Quality quality, Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize, const Job& job = nullptr);

		static ByteArray SRK_CALL encode(const Image& img, const Vector<3, uint8_t>& blockSize, Profile profile, Quality quality, Flags flags, size_t threadCount, const Job& job = nullptr);
	};
}