#pragma once

#include "aurora/ByteArray.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include "ASTCConverter.h"
#include "astcenc.h"

#include <thread>

namespace aurora::extensions::astc_converter {
	void AE_CALL release(astcenc_context* context) {
		if (context) astcenc_context_free(context);
	}

	ByteArray AE_CALL encode(const Image& img, Vec3<uint8_t> blockSize, ASTCConverter::Preset preset, ASTCConverter::Flags flags, size_t threadCount) {
		using namespace enum_operators;

		if (img.format != modules::graphics::TextureFormat::R8G8B8A8) return ByteArray();

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
		astcenc_profile profile = ASTCENC_PRF_LDR;

		if (astcenc_config_init(profile, blockSize[0], blockSize[1], blockSize[2], presetVal, (uint32_t)flags, &config) != ASTCENC_SUCCESS) {
			return ByteArray();
		}

		astcenc_context* context = nullptr;
		if (auto err = astcenc_context_alloc(&config, threadCount, &context); err != ASTCENC_SUCCESS) {
			release(context);
			return ByteArray();
		}

		constexpr size_t HEADER_LEN = 16;

		Vec3<size_t> blocks((in.dim_x + config.block_x - 1) / config.block_x, (in.dim_y + config.block_y - 1) / config.block_y, (in.dim_z + config.block_z - 1) / config.block_z);
		auto bufferSize = blocks.getMultiplies() * 16 + HEADER_LEN;
		auto buffer = new uint8_t[bufferSize];
		auto outData = buffer + HEADER_LEN;
		auto outSize = bufferSize - HEADER_LEN;
		{
			const uint32_t ASTC_MAGIC_ID = 0x5CA1AB13;

			buffer[0] = ASTC_MAGIC_ID & 0xFF;
			buffer[1] = (ASTC_MAGIC_ID >> 8) & 0xFF;
			buffer[2] = (ASTC_MAGIC_ID >> 16) & 0xFF;
			buffer[3] = (ASTC_MAGIC_ID >> 24) & 0xFF;

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
				threads[i] = std::thread([context, &in, &swizzle, outData, outSize, i, &error]() {
					if (auto err = astcenc_compress_image(context, &in, &swizzle, outData, outSize, i); err != ASTCENC_SUCCESS) error = err;
				});
			}

			for (decltype(threadCount) i = 0; i < threadCount; ++i) threads[i].join();
			delete[] threads;
		} else {
			error = astcenc_compress_image(context, &in, &swizzle, outData, outSize, 0);
		}

		if (error != ASTCENC_SUCCESS) {
			printf("ERROR: Codec compress failed: %s\n", astcenc_get_error_string(error));
			release(context);
			delete[] buffer;
			return ByteArray();
		}

		release(context);
		return ByteArray(buffer, bufferSize, ByteArray::Usage::EXCLUSIVE);
	}
}