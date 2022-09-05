#pragma once

#include "srk/ByteArray.h"
#include "srk/Thread.h"
#include "srk/modules/graphics/IGraphicsModule.h"
#include "ASTCConverter.h"
#include "astcenc.h"

#include <thread>

namespace srk::extensions::astc_converter {
	class Impl {
	public:
		static bool SRK_CALL encode(const Image& img, const Vec3<uint8_t>& blockSize, ASTCConverter::Profile profile, ASTCConverter::Quality quality, ASTCConverter::Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize, const ASTCConverter::Job& job) {
			using namespace enum_operators;

			if (img.format != modules::graphics::TextureFormat::R8G8B8A8) return false;

			if (threadCount == 0) threadCount = 1;

			auto data = (void*)img.source.getSource();

			astcenc_image in;
			in.dim_x = img.size[0];
			in.dim_y = img.size[1];
			in.dim_z = 1;
			in.data_type = ASTCENC_TYPE_U8;
			in.data = &data;

			float32_t qlty;
			switch (quality) {
			case ASTCConverter::Quality::FAST:
				qlty = ASTCENC_PRE_FAST;
				break;
			case ASTCConverter::Quality::MEDIUM:
				qlty = ASTCENC_PRE_MEDIUM;
				break;
			case ASTCConverter::Quality::THOROUGH:
				qlty = ASTCENC_PRE_THOROUGH;
				break;
			case ASTCConverter::Quality::EXHAUSTIVE:
				qlty = ASTCENC_PRE_EXHAUSTIVE;
				break;
			default:
				qlty = ASTCENC_PRE_FASTEST;
				break;
			}

			astcenc_config config{};
			if (astcenc_config_init((astcenc_profile)profile, blockSize[0], blockSize[1], blockSize[2], qlty, ((uint32_t)flags & 0x7F), &config) != ASTCENC_SUCCESS) return false;

			Vec3<size_t> blocks((in.dim_x + config.block_x - 1) / config.block_x, (in.dim_y + config.block_y - 1) / config.block_y, (in.dim_z + config.block_z - 1) / config.block_z);
			auto numBlocks = blocks.getMultiplies();
			auto needBufferSize = numBlocks * 16;
			auto isWriteHeader = (flags & ASTCConverter::Flags::WRITE_HEADER) == ASTCConverter::Flags::WRITE_HEADER;
			if (isWriteHeader) needBufferSize += ASTCConverter::HEADER_SIZE;

			if (outBuffer == nullptr) {
				outBufferSize = needBufferSize;
				return true;
			}

			threadCount = Thread::calcNeedCount(numBlocks, 1, threadCount);
			if (!threadCount) return true;
			
			uint8_t* buffer = nullptr;
			if (*outBuffer == nullptr) {
				buffer = new uint8_t[needBufferSize];
			} else if (needBufferSize > outBufferSize) {
				outBufferSize = needBufferSize;
				return false;
			} else {
				outBufferSize = needBufferSize;
				buffer = (uint8_t*)*outBuffer;
			}

			astcenc_context* context = nullptr;
			if (auto err = astcenc_context_alloc(&config, threadCount, &context); err != ASTCENC_SUCCESS) return false;

			uint8_t* dataBuffer = buffer;
			size_t dataBufferSize = needBufferSize;
			if (isWriteHeader) {
				dataBuffer += ASTCConverter::HEADER_SIZE;
				dataBufferSize -= ASTCConverter::HEADER_SIZE;
			}

			astcenc_swizzle swizzle{ ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A };
			if ((flags & ASTCConverter::Flags::MAP_NORMAL) == ASTCConverter::Flags::MAP_NORMAL) {
				swizzle.r = ASTCENC_SWZ_R;
				swizzle.g = ASTCENC_SWZ_R;
				swizzle.b = ASTCENC_SWZ_R;
				swizzle.a = ASTCENC_SWZ_G;
			}

			std::atomic_int32_t error = ASTCENC_SUCCESS;
			if (threadCount > 1) {
				std::thread* threads = nullptr;
				std::shared_future<void>* futures = nullptr;
				if (job) {
					futures = new std::shared_future<void>[threadCount - 1];
					for (decltype(threadCount) i = 1; i < threadCount; ++i) {
						futures[i - 1] = job([context, &in, &swizzle, dataBuffer, dataBufferSize, i, &error]() {
							if (auto err = astcenc_compress_image(context, &in, &swizzle, dataBuffer, dataBufferSize, i); err != ASTCENC_SUCCESS) error = err;
							});
					}
				} else {
					threads = new std::thread[threadCount - 1];
					for (decltype(threadCount) i = 1; i < threadCount; ++i) {
						threads[i - 1] = std::thread([context, &in, &swizzle, dataBuffer, dataBufferSize, i, &error]() {
							if (auto err = astcenc_compress_image(context, &in, &swizzle, dataBuffer, dataBufferSize, i); err != ASTCENC_SUCCESS) error = err;
							});
					}
				}

				if (auto err = astcenc_compress_image(context, &in, &swizzle, dataBuffer, dataBufferSize, 0); err != ASTCENC_SUCCESS) error = err;

				if (job) {
					for (decltype(threadCount) i = 1; i < threadCount; ++i) futures[i - 1].wait();
					delete[] futures;
				} else {
					for (decltype(threadCount) i = 1; i < threadCount; ++i) threads[i - 1].join();
					delete[] threads;
				}
			} else {
				error = astcenc_compress_image(context, &in, &swizzle, dataBuffer, dataBufferSize, 0);
			}

			if (error != ASTCENC_SUCCESS) {
				printf("ERROR: Codec compress failed: %s\n", astcenc_get_error_string((astcenc_error)error.load()));
				astcenc_context_free(context);
				if (buffer != *outBuffer) delete[] buffer;
				return false;
			}

			astcenc_context_free(context);

			if (isWriteHeader) {
				buffer[0] = ASTCConverter::HEADER_MAGIC_ID & 0xFF;
				buffer[1] = (ASTCConverter::HEADER_MAGIC_ID >> 8) & 0xFF;
				buffer[2] = (ASTCConverter::HEADER_MAGIC_ID >> 16) & 0xFF;
				buffer[3] = (ASTCConverter::HEADER_MAGIC_ID >> 24) & 0xFF;

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