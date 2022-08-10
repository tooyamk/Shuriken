#pragma once

#include "srk/ByteArray.h"
#include "srk/modules/graphics/IGraphicsModule.h"
#include "ASTCConverter.h"
#include "astcenc.h"

#include <thread>

namespace srk::extensions::astc_converter {
	class Impl {
	public:
		static constexpr size_t HEADER_SIZE = 16;
		static constexpr uint32_t HEADER_MAGIC_ID = 0x5CA1AB13;

		static bool SRK_CALL encode(const Image& img, const Vec3<uint8_t>& blockSize, ASTCConverter::Profile profile, ASTCConverter::Preset preset, ASTCConverter::Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize) {
			using namespace enum_operators;

			outBufferSize = 0;

			if (img.format != modules::graphics::TextureFormat::R8G8B8A8) return false;

			if (threadCount == 0) threadCount = 1;

			auto data = (void*)img.source.getSource();

			astcenc_image in;
			in.dim_x = img.size[0];
			in.dim_y = img.size[1];
			in.dim_z = 1;
			in.data_type = ASTCENC_TYPE_U8;
			in.data = &data;

			float32_t presetVal;
			switch (preset) {
			case ASTCConverter::Preset::FAST:
				presetVal = ASTCENC_PRE_FAST;
				break;
			case ASTCConverter::Preset::MEDIUM:
				presetVal = ASTCENC_PRE_MEDIUM;
				break;
			case ASTCConverter::Preset::THOROUGH:
				presetVal = ASTCENC_PRE_THOROUGH;
				break;
			case ASTCConverter::Preset::EXHAUSTIVE:
				presetVal = ASTCENC_PRE_EXHAUSTIVE;
				break;
			default:
				presetVal = ASTCENC_PRE_FASTEST;
				break;
			}

			astcenc_config config{};
			if (astcenc_config_init((astcenc_profile)profile, blockSize[0], blockSize[1], blockSize[2], presetVal, ((uint32_t)flags & 0x7F), &config) != ASTCENC_SUCCESS) return false;

			auto isWriteHeader = (flags & ASTCConverter::Flags::WRITE_HEADER) == ASTCConverter::Flags::WRITE_HEADER;
			Vec3<size_t> blocks((in.dim_x + config.block_x - 1) / config.block_x, (in.dim_y + config.block_y - 1) / config.block_y, (in.dim_z + config.block_z - 1) / config.block_z);
			auto needBufferSize = blocks.getMultiplies() * 16;
			if (isWriteHeader) needBufferSize += HEADER_SIZE;

			if (outBuffer == nullptr) {
				outBufferSize = needBufferSize;
				return true;
			}

			uint8_t* buffer = nullptr;
			if (*outBuffer == nullptr) {
				buffer = new uint8_t[needBufferSize];
			} else if (needBufferSize > outBufferSize) {
				return false;
			} else {
				buffer = (uint8_t*)*outBuffer;
			}

			astcenc_context* context = nullptr;
			if (auto err = astcenc_context_alloc(&config, threadCount, &context); err != ASTCENC_SUCCESS) return false;

			uint8_t* dataBuffer = buffer;
			size_t dataBufferSize = needBufferSize;
			if (isWriteHeader) {
				dataBuffer += HEADER_SIZE;
				dataBufferSize -= HEADER_SIZE;
			}

			astcenc_swizzle swizzle{ ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A };
			if ((flags & ASTCConverter::Flags::MAP_NORMAL) == ASTCConverter::Flags::MAP_NORMAL) {
				swizzle.r = ASTCENC_SWZ_R;
				swizzle.g = ASTCENC_SWZ_R;
				swizzle.b = ASTCENC_SWZ_R;
				swizzle.a = ASTCENC_SWZ_G;
			}

			auto error = ASTCENC_SUCCESS;
			if (threadCount > 1) {
				auto threads = new std::thread[threadCount];
				for (decltype(threadCount) i = 0; i < threadCount; ++i) {
					threads[i] = std::thread([context, &in, &swizzle, dataBuffer, dataBufferSize, i, &error]() {
						if (auto err = astcenc_compress_image(context, &in, &swizzle, dataBuffer, dataBufferSize, i); err != ASTCENC_SUCCESS) error = err;
						});
				}

				for (decltype(threadCount) i = 0; i < threadCount; ++i) threads[i].join();
				delete[] threads;
			} else {
				error = astcenc_compress_image(context, &in, &swizzle, dataBuffer, dataBufferSize, 0);
			}

			if (error != ASTCENC_SUCCESS) {
				printf("ERROR: Codec compress failed: %s\n", astcenc_get_error_string(error));
				astcenc_context_free(context);
				if (buffer != *outBuffer) delete[] buffer;
				return false;
			}

			astcenc_context_free(context);

			if (isWriteHeader) {
				buffer[0] = HEADER_MAGIC_ID & 0xFF;
				buffer[1] = (HEADER_MAGIC_ID >> 8) & 0xFF;
				buffer[2] = (HEADER_MAGIC_ID >> 16) & 0xFF;
				buffer[3] = (HEADER_MAGIC_ID >> 24) & 0xFF;

				buffer[4] = config.block_x;
				buffer[5] = config.block_y;
				buffer[6] = config.block_z;

				buffer[7] = in.dim_x & 0xFF;
				buffer[8] = (in.dim_x >> 8) & 0xFF;
				buffer[9] = (in.dim_x >> 16) & 0xFF;

				buffer[10] = in.dim_y & 0xFF;
				buffer[11] = (in.dim_y >> 8) & 0xFF;
				buffer[12] = (in.dim_y >> 16) & 0xFF;

				buffer[13] = in.dim_z & 0xFF;
				buffer[14] = (in.dim_z >> 8) & 0xFF;
				buffer[15] = (in.dim_z >> 16) & 0xFF;
			}

			*outBuffer = buffer;
			outBufferSize = needBufferSize;

			return true;
		}
	};
}