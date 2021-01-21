#pragma once

#include "aurora/ByteArray.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include "ASTCConverter.h"
#include "astcenc.h"
#include "astcenc_lib.h"

#include <vector>

namespace aurora::extensions::astc_converter {
	ByteArray AE_CALL encode(const Image& img, ASTCConverter::BlockSize blockSize, ASTCConverter::Preset preset, size_t threadCount) {
		if (img.format != modules::graphics::TextureFormat::R8G8B8A8) return ByteArray();

		std::vector<std::string> opts;
		opts.emplace_back("-silent");
		if (threadCount) {
			opts.emplace_back("-j");
			opts.emplace_back(std::to_string(threadCount));
		}

		auto data = (void*)img.source.getSource();

		astcenc_image in;
		in.dim_x = img.size[0];
		in.dim_y = img.size[1];
		in.dim_z = 1;
		in.data_type = ASTCENC_TYPE_U8;
		in.data = &data;

		uint8_t* outData = nullptr;
		size_t outSize;

		std::string blockSizeVal;
		switch (blockSize) {
		case ASTCConverter::BlockSize::BLOCK_4x4:
			blockSizeVal = "4x4";
			break;
		case ASTCConverter::BlockSize::BLOCK_5x4:
			blockSizeVal = "5x4";
			break;
		case ASTCConverter::BlockSize::BLOCK_5x5:
			blockSizeVal = "5x5";
			break;
		case ASTCConverter::BlockSize::BLOCK_6x5:
			blockSizeVal = "6x5";
			break;
		case ASTCConverter::BlockSize::BLOCK_6x6:
			blockSizeVal = "6x6";
			break;
		case ASTCConverter::BlockSize::BLOCK_8x5:
			blockSizeVal = "8x5";
			break;
		case ASTCConverter::BlockSize::BLOCK_8x6:
			blockSizeVal = "8x6";
			break;
		case ASTCConverter::BlockSize::BLOCK_10x5:
			blockSizeVal = "10x5";
			break;
		case ASTCConverter::BlockSize::BLOCK_10x6:
			blockSizeVal = "10x6";
			break;
		case ASTCConverter::BlockSize::BLOCK_8x8:
			blockSizeVal = "8x8";
			break;
		case ASTCConverter::BlockSize::BLOCK_10x8:
			blockSizeVal = "10x8";
			break;
		case ASTCConverter::BlockSize::BLOCK_10x10:
			blockSizeVal = "10x10";
			break;
		case ASTCConverter::BlockSize::BLOCK_12x10:
			blockSizeVal = "12x10";
			break;
		case ASTCConverter::BlockSize::BLOCK_12x12:
			blockSizeVal = "12x12";
			break;
		case ASTCConverter::BlockSize::BLOCK_3x3x3:
			blockSizeVal = "3x3x3";
			break;
		case ASTCConverter::BlockSize::BLOCK_4x3x3:
			blockSizeVal = "4x3x3";
			break;
		case ASTCConverter::BlockSize::BLOCK_4x4x3:
			blockSizeVal = "4x4x3";
			break;
		case ASTCConverter::BlockSize::BLOCK_4x4x4:
			blockSizeVal = "4x4x4";
			break;
		case ASTCConverter::BlockSize::BLOCK_5x4x4:
			blockSizeVal = "5x4x4";
			break;
		case ASTCConverter::BlockSize::BLOCK_5x5x4:
			blockSizeVal = "5x5x4";
			break;
		case ASTCConverter::BlockSize::BLOCK_5x5x5:
			blockSizeVal = "5x5x5";
			break;
		case ASTCConverter::BlockSize::BLOCK_6x5x5:
			blockSizeVal = "6x5x5";
			break;
		case ASTCConverter::BlockSize::BLOCK_6x6x5:
			blockSizeVal = "6x6x5";
			break;
		case ASTCConverter::BlockSize::BLOCK_6x6x6:
			blockSizeVal = "6x6x6";
			break;
		default:
			break;
		}

		std::string presetVal;
		switch (preset) {
		case ASTCConverter::Preset::FASTEST:
			presetVal = "-fastest";
			break;
		case ASTCConverter::Preset::FAST:
			presetVal = "-fast";
			break;
		case ASTCConverter::Preset::MEDIUM:
			presetVal = "-medium";
			break;
		case ASTCConverter::Preset::THOROUGH:
			presetVal = "-thorough";
			break;
		case ASTCConverter::Preset::EXHAUSTIVE:
			presetVal = "-exhaustive";
			break;
		default:
			break;
		}

		if (astcenc_lib::encode("-cl", blockSizeVal, presetVal, opts, in, outData, outSize) == 0) {
			return ByteArray(outData, outSize, ByteArray::Usage::EXCLUSIVE);
		} else {
			if (outData) delete[] outData;
			return ByteArray();
		}

		return ByteArray();
	}
}