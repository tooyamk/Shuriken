#pragma once

#include "srk/ByteArray.h"
#include "srk/modules/graphics/IGraphicsModule.h"
#include "BC7Converter.h"
#include "bc7enc.h"

#include <thread>

namespace srk::extensions::bc7_converter {
	class Impl {
	public:
		static bool SRK_CALL encode(const Image& img, uint32_t uberLevel, uint32_t maxPartitionsToScan, BC7Converter::Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize) {
			using namespace enum_operators;

			if (img.format != modules::graphics::TextureFormat::R8G8B8A8) return false;
			if (img.size[0] % 4 != 0 || img.size[1] % 4 != 0) return false;

			if (threadCount == 0) threadCount = 1;

			Config cfg;
			cfg.size = img.size;
			cfg.blocks = cfg.size >> 2;
			cfg.numBlocks = cfg.blocks.getMultiplies();

			auto needBufferSize = cfg.numBlocks * 16;
			auto isWriteHeader = (flags & BC7Converter::Flags::WRITE_DDS_HEADER) == BC7Converter::Flags::WRITE_DDS_HEADER;
			if (isWriteHeader) needBufferSize += BC7Converter::DDS_HEADER_SIZE;

			if (outBuffer == nullptr) {
				outBufferSize = needBufferSize;
				return true;
			}

			cfg.numBlocksPerThread = (cfg.numBlocks + threadCount - 1) / threadCount;
			cfg.in = img.source.getSource();

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

			cfg.out = buffer;
			size_t dataBufferSize = needBufferSize;
			if (isWriteHeader) {
				cfg.out += BC7Converter::DDS_HEADER_SIZE;
				dataBufferSize -= BC7Converter::DDS_HEADER_SIZE;
			}

			bc7enc_compress_block_params_init(&cfg.params);
			if ((flags & BC7Converter::Flags::PERCEPTUAL) == BC7Converter::Flags::NONE) bc7enc_compress_block_params_init_linear_weights(&cfg.params);
			cfg.params.m_max_partitions_mode = std::min(maxPartitionsToScan, (uint32_t)BC7ENC_MAX_PARTITIONS1);
			cfg.params.m_uber_level = std::min(uberLevel, (uint32_t)BC7ENC_MAX_UBER_LEVEL);

			bc7enc_compress_block_init();

			auto numBlocksPerThread = (cfg.numBlocks + threadCount - 1) / threadCount;

			if (threadCount > 1) {
				auto threads = new std::thread[threadCount - 1];
				for (decltype(threadCount) i = 1; i < threadCount; ++i) {
					threads[i - 1] = std::thread([&cfg, i]() {
						_encode(cfg, i);
					});
				}

				_encode(cfg, 0);

				for (decltype(threadCount) i = 1; i < threadCount; ++i) threads[i - 1].join();
				delete[] threads;
			} else {
				_encode(cfg, 0);
			}

			if (isWriteHeader) _writeDdsHeader(buffer, cfg);

			*outBuffer = buffer;
			outBufferSize = needBufferSize;

			return true;
		}

	private:
		struct Config {
			Vec2<size_t> size;
			Vec2<size_t> blocks;
			size_t numBlocks;

			size_t numBlocksPerThread;
			const uint8_t* in;
			uint8_t* out;

			bc7enc_compress_block_params params;
		};

		static void SRK_CALL _encode(const Config& cfg, uint32_t threadIndex) {
			auto startBlock = cfg.numBlocksPerThread * threadIndex;
			auto endBlock = startBlock + cfg.numBlocksPerThread;
			if (endBlock > cfg.numBlocks) endBlock = cfg.numBlocks;

			uint8_t in[16 << 2];
			auto in32 = (uint32_t*)in;
			while (startBlock < endBlock) {
				auto by = startBlock / cfg.blocks[0];
				auto bx = startBlock - by * cfg.blocks[0];

				auto px = bx << 4;
				auto py = by << 2;
				auto bytesPerRow = cfg.size[0] * 4;

				for (auto y = 0; y < 4; ++y) memcpy(in + (y << 4), cfg.in + (py + y) * bytesPerRow + px, 16);

				bc7enc_compress_block(cfg.out + (startBlock << 4), in, &cfg.params);

				++startBlock;
			}
		}

		static void SRK_CALL _writeDdsHeader(uint8_t* out, const Config& cfg) {
			ByteArray ba(out, BC7Converter::DDS_HEADER_SIZE);
			ba.write<uint32_t>(BC7Converter::DDS_HEADER_MAGIC_ID);

			//DDS_HEADER
			{
				ba.write<uint32_t>(124);//dwSize
				ba.write<uint32_t>(0x1000 | 0x4 | 0x2 | 0x1);//dwFlags
				ba.write<uint32_t>(cfg.size[1]);//dwHeight
				ba.write<uint32_t>(cfg.size[0]);//dwWidth
				ba.write<uint32_t>(cfg.size.getMultiplies());//dwPitchOrLinearSize
				ba.write<uint32_t>(0);//dwDepth
				ba.write<uint32_t>(0);//dwMipMapCount
				for (auto i = 0; i < 11; ++i) ba.write<uint32_t>(0);//dwReserved1

				//DDS_PIXELFORMAT
				{
					ba.write<uint32_t>(32);//dwSize
					ba.write<uint32_t>(0x4);//dwFlags
					ba.write('D');//dwFourCC
					ba.write('X');
					ba.write('1');
					ba.write('0');
					ba.write<uint32_t>(0);//dwRGBBitCount
					ba.write<uint32_t>(0);//dwRBitMask
					ba.write<uint32_t>(0);//dwGBitMask
					ba.write<uint32_t>(0);//dwBBitMask
					ba.write<uint32_t>(0);//dwABitMask
				}

				ba.write<uint32_t>(0x1000);//dwCaps
				ba.write<uint32_t>(0);//dwCaps2
				ba.write<uint32_t>(0);//dwCaps3
				ba.write<uint32_t>(0);//dwCaps4
				ba.write<uint32_t>(0);//dwReserved2
			}

			//DDS_HEADER_DXT10
			{
				ba.write<uint32_t>((uint32_t)98);//dwCaps2 DXGI_FORMAT_BC7_UNORM
				ba.write<uint32_t>(3);//D3D10_RESOURCE_DIMENSION D3D10_RESOURCE_DIMENSION_TEXTURE2D
				ba.write<uint32_t>(0);//miscFlag
				ba.write<uint32_t>(1);//arraySize
				ba.write<uint32_t>(0);//miscFlags2
			}
		}
	};
}