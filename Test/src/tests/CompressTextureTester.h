#pragma once

#include "../BaseTester.h"
#include "srk/math/Math.h"

#include <random>

namespace srk::extensions {
	class BC7Converter {
	public:
		static constexpr size_t DDS_HEADER_SIZE = 148;
		static constexpr uint32_t HEADER_MAGIC_ID = 0x20534444;

		enum class Profile : uint8_t {
			TYPELESS = 97,
			UNORM,
			UNORM_SRGB
		};

		enum class Quality : uint8_t {
			Fast,
			Balanced,
			Best
		};

		enum class Flags : uint8_t {
			NONE = 0,
			WRITE_DDS_HEADER = 1 << 0
		};

		struct Config {
			Profile prefile;
			Vec2<size_t> size;
			Vec2<size_t> blocks;
			Quality quality;
			size_t numBlocks;
			size_t numBlocksPerThread;
			const uint8_t* data;
		};

		static ByteArray SRK_CALL encode(const Image& img, Profile profile, Quality quality, Flags flags, size_t threadCount) {
			void* buffer = nullptr;
			size_t bufferSize;
			if (!encode(img, profile, quality, flags, threadCount, &buffer, bufferSize)) return ByteArray();

			return ByteArray(buffer, bufferSize, ByteArray::Usage::EXCLUSIVE);
		}

		static bool SRK_CALL encode(const Image& img, Profile profile, Quality quality, Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize) {
			using namespace enum_operators;

			outBufferSize = 0;

			if (img.format != modules::graphics::TextureFormat::R8G8B8A8) return false;
			if (!std::has_single_bit(img.size[0]) || !std::has_single_bit(img.size[1])) return false;

			if (threadCount == 0) threadCount = 1;

			Config cfg;
			cfg.size = img.size;
			cfg.blocks = cfg.size >> 2;
			cfg.numBlocks = cfg.blocks.getMultiplies();
			cfg.quality = quality;
			auto needBufferSize = cfg.numBlocks * 16;

			auto isWriteHeader = (flags & BC7Converter::Flags::WRITE_DDS_HEADER) == BC7Converter::Flags::WRITE_DDS_HEADER;
			if (isWriteHeader) needBufferSize += DDS_HEADER_SIZE;

			if (outBuffer == nullptr) {
				outBufferSize = needBufferSize;
				return true;
			}

			cfg.numBlocksPerThread = (cfg.numBlocks + threadCount - 1) / threadCount;
			cfg.data = img.source.getSource();
			cfg.prefile = profile;

			uint8_t* buffer = nullptr;
			if (*outBuffer == nullptr) {
				buffer = new uint8_t[needBufferSize];
			} else if (needBufferSize > outBufferSize) {
				return false;
			} else {
				buffer = (uint8_t*)*outBuffer;
			}

			uint8_t* dataBuffer = buffer;
			size_t dataBufferSize = needBufferSize;
			if (isWriteHeader) {
				dataBuffer += DDS_HEADER_SIZE;
				dataBufferSize -= DDS_HEADER_SIZE;
			}

			if (threadCount > 1) {
				auto threads = new std::thread[threadCount - 1];
				for (decltype(threadCount) i = 1; i < threadCount; ++i) {
					threads[i - 1] = std::thread([&cfg, dataBuffer, i]() {
						_encode(cfg, dataBuffer, i);
						});
				}

				_encode(cfg, dataBuffer, 0);

				for (decltype(threadCount) i = 1; i < threadCount; ++i) threads[i - 1].join();
				delete[] threads;
			} else {
				_encode(cfg, dataBuffer, 0);
			}

			if (isWriteHeader) _writeDdsHeader(buffer, cfg);

			*outBuffer = buffer;
			outBufferSize = needBufferSize;

			return true;
		}

	private:
		enum class BlockMode : uint8_t {
			NONE = 0,
			ZERO = 1 << 0,
			ONE = 1 << 1,
			TWO = 1 << 2,
			THREE = 1 << 3,
			FOUR = 1 << 4,
			FIVE = 1 << 5,
			SIX = 1 << 6,
			SEVEN = 1 << 7,

			TWO_PARTITION_MODES = ONE | THREE | SEVEN,
			THREE_PARTITION_MODES = ZERO | TWO,
			ALPHA_MODES = FOUR | FIVE | SIX | SEVEN
		};

		struct Shape {
			uint32_t numPartitions;
			uint32_t index;
		};

		struct ShapeSelection {
			std::vector<Shape> shapes;
			BlockMode selectedModes;

			ShapeSelection()
				: selectedModes((BlockMode)(0xFF)) {}
		};

		enum class ErrorMetric : uint8_t {
			UNIFORM,     // Treats r, g, and b channels equally
			NONUNIFORM,  // { 0.3, 0.59, 0.11 }
		};

		enum class PBitType : uint8_t {
			NONE,
			SHARED,
			UNIQUE
		};

		struct ModeInfo {
			uint8_t partitions;
			uint8_t partitionBits;
			uint8_t pBits;
			uint8_t rotationBits;
			uint8_t indexModeBits;
			uint8_t indexPrec1;
			uint8_t indexPrec2;
			Vec4ui8 rgbaPrec;
			Vec4ui8 rgbaPrecWithP;
			PBitType pBitType;
		};

		static constexpr uint32_t MAX_REGIONS = 3;
		static constexpr uint32_t NUM_CHANNELS = 4;
		static constexpr uint32_t MAX_SHAPES = 64;

		static constexpr uint32_t WEIGHT_MAX = 64;
		static constexpr uint32_t WEIGHT_SHIFT = 6;
		static constexpr uint32_t WEIGHT_ROUND = 32;

		static constexpr uint32_t MAX_NUM_DATA_POINTS = 16;

		static constexpr uint8_t O_MATCH6[256][2] = { {0, 0}, {0, 1}, {1, 0}, {1, 0}, {1, 1}, {2, 0}, {2, 0}, {2, 1}, {3, 0}, {3, 1}, {4, 0}, {4, 0}, {4, 1}, {5, 0}, {5, 0}, {5, 1}, {6, 0}, {6, 1}, {7, 0}, {7, 0}, {7, 1}, {8, 0}, {0, 16}, {8, 1}, {9, 0}, {9, 1}, {1, 17}, {10, 0}, {10, 1}, {11, 0}, {3, 16}, {11, 1}, {12, 0}, {12, 1}, {4, 17}, {13, 0}, {13, 1}, {14, 0}, {6, 16}, {14, 1}, {15, 0}, {15, 1}, {7, 17}, {16, 0}, {15, 3}, {16, 1}, {17, 0}, {17, 1}, {15, 6}, {18, 0}, {18, 1}, {19, 0}, {15, 9}, {19, 1}, {20, 0}, {20, 1}, {15, 12}, {21, 0}, {21, 1}, {22, 0}, {15, 15}, {22, 1}, {23, 0}, {23, 1}, {15, 18}, {24, 0}, {24, 1}, {25, 0}, {17, 16}, {25, 1}, {26, 0}, {26, 1}, {18, 17}, {27, 0}, {27, 1}, {28, 0}, {20, 16}, {28, 1}, {29, 0}, {29, 1}, {21, 17}, {30, 0}, {30, 1}, {31, 0}, {23, 16}, {31, 1}, {31, 2}, {32, 0}, {32, 1}, {33, 0}, {31, 5}, {33, 1}, {34, 0}, {34, 1}, {31, 8}, {35, 0}, {35, 1}, {36, 0}, {31, 11}, {36, 1}, {37, 0}, {37, 1}, {31, 14}, {38, 0}, {38, 1}, {39, 0}, {31, 17}, {39, 1}, {40, 0}, {40, 1}, {32, 17}, {41, 0}, {41, 1}, {42, 0}, {34, 16}, {42, 1}, {43, 0}, {43, 1}, {35, 17}, {44, 0}, {44, 1}, {45, 0}, {37, 16}, {45, 1}, {46, 0}, {46, 1}, {38, 17}, {47, 0}, {47, 1}, {47, 2}, {48, 0}, {48, 1}, {47, 4}, {49, 0}, {49, 1}, {50, 0}, {47, 7}, {50, 1}, {51, 0}, {51, 1}, {47, 10}, {52, 0}, {52, 1}, {53, 0}, {47, 13}, {53, 1}, {54, 0}, {54, 1}, {47, 16}, {55, 0}, {55, 1}, {56, 0}, {48, 16}, {56, 1}, {57, 0}, {57, 1}, {49, 17}, {58, 0}, {58, 1}, {59, 0}, {51, 16}, {59, 1}, {60, 0}, {60, 1}, {52, 17}, {61, 0}, {61, 1}, {62, 0}, {54, 16}, {62, 1}, {63, 0}, {63, 1}, {55, 17}, {63, 2}, {63, 3}, {63, 4}, {57, 16}, {63, 5}, {63, 6}, {63, 7}, {58, 17}, {63, 8}, {63, 9}, {63, 10}, {60, 16}, {63, 11}, {63, 12}, {63, 13}, {61, 17}, {63, 14}, {63, 15}, {54, 33}, {63, 16}, {63, 17}, {63, 18}, {56, 32}, {63, 19}, {63, 20}, {63, 21}, {57, 33}, {63, 22}, {63, 23}, {63, 24}, {59, 32}, {63, 25}, {63, 26}, {63, 27}, {60, 33}, {63, 28}, {63, 29}, {63, 30}, {62, 32}, {63, 31}, {63, 32}, {55, 48}, {63, 33}, {63, 34}, {63, 35}, {56, 49}, {63, 36}, {63, 37}, {63, 38}, {58, 48}, {63, 39}, {63, 40}, {63, 41}, {59, 49}, {63, 42}, {63, 43}, {63, 44}, {61, 48}, {63, 45}, {63, 46}, {63, 47}, {62, 49}, {63, 48}, {63, 49}, {63, 49}, {63, 50}, {63, 51}, {63, 52}, {63, 52}, {63, 53}, {63, 54}, {63, 55}, {63, 55}, {63, 56}, {63, 57}, {63, 58}, {63, 58}, {63, 59}, {63, 60}, {63, 61}, {63, 61}, {63, 62}, {63, 63} };
		static constexpr uint8_t O_MATCH7[256][2] = { {0, 0}, {0, 1}, {0, 3}, {0, 4}, {0, 6}, {0, 7}, {0, 9}, {0, 10}, {0, 12}, {0, 13}, {0, 15}, {0, 16}, {0, 18}, {0, 20}, {0, 21}, {0, 23}, {0, 24}, {0, 26}, {0, 27}, {0, 29}, {0, 30}, {0, 32}, {0, 33}, {0, 35}, {0, 36}, {0, 38}, {0, 39}, {0, 41}, {0, 42}, {0, 44}, {0, 45}, {0, 47}, {0, 48}, {0, 50}, {0, 52}, {0, 53}, {0, 55}, {0, 56}, {0, 58}, {0, 59}, {0, 61}, {0, 62}, {0, 64}, {0, 65}, {0, 66}, {0, 68}, {0, 69}, {0, 71}, {0, 72}, {0, 74}, {0, 75}, {0, 77}, {0, 78}, {0, 80}, {0, 82}, {0, 83}, {0, 85}, {0, 86}, {0, 88}, {0, 89}, {0, 91}, {0, 92}, {0, 94}, {0, 95}, {0, 97}, {0, 98}, {0, 100}, {0, 101}, {0, 103}, {0, 104}, {0, 106}, {0, 107}, {0, 109}, {0, 110}, {0, 112}, {0, 114}, {0, 115}, {0, 117}, {0, 118}, {0, 120}, {0, 121}, {0, 123}, {0, 124}, {0, 126}, {0, 127}, {1, 127}, {2, 126}, {3, 126}, {3, 127}, {4, 127}, {5, 126}, {6, 126}, {6, 127}, {7, 127}, {8, 126}, {9, 126}, {9, 127}, {10, 127}, {11, 126}, {12, 126}, {12, 127}, {13, 127}, {14, 126}, {15, 125}, {15, 127}, {16, 126}, {17, 126}, {17, 127}, {18, 127}, {19, 126}, {20, 126}, {20, 127}, {21, 127}, {22, 126}, {23, 126}, {23, 127}, {24, 127}, {25, 126}, {26, 126}, {26, 127}, {27, 127}, {28, 126}, {29, 126}, {29, 127}, {30, 127}, {31, 126}, {32, 126}, {32, 127}, {33, 127}, {34, 126}, {35, 126}, {35, 127}, {36, 127}, {37, 126}, {38, 126}, {38, 127}, {39, 127}, {40, 126}, {41, 126}, {41, 127}, {42, 127}, {43, 126}, {44, 126}, {44, 127}, {45, 127}, {46, 126}, {47, 125}, {47, 127}, {48, 126}, {49, 126}, {49, 127}, {50, 127}, {51, 126}, {52, 126}, {52, 127}, {53, 127}, {54, 126}, {55, 126}, {55, 127}, {56, 127}, {57, 126}, {58, 126}, {58, 127}, {59, 127}, {60, 126}, {61, 126}, {61, 127}, {62, 127}, {63, 126}, {64, 125}, {64, 126}, {65, 126}, {65, 127}, {66, 127}, {67, 126}, {68, 126}, {68, 127}, {69, 127}, {70, 126}, {71, 126}, {71, 127}, {72, 127}, {73, 126}, {74, 126}, {74, 127}, {75, 127}, {76, 126}, {77, 125}, {77, 127}, {78, 126}, {79, 126}, {79, 127}, {80, 127}, {81, 126}, {82, 126}, {82, 127}, {83, 127}, {84, 126}, {85, 126}, {85, 127}, {86, 127}, {87, 126}, {88, 126}, {88, 127}, {89, 127}, {90, 126}, {91, 126}, {91, 127}, {92, 127}, {93, 126}, {94, 126}, {94, 127}, {95, 127}, {96, 126}, {97, 126}, {97, 127}, {98, 127}, {99, 126}, {100, 126}, {100, 127}, {101, 127}, {102, 126}, {103, 126}, {103, 127}, {104, 127}, {105, 126}, {106, 126}, {106, 127}, {107, 127}, {108, 126}, {109, 125}, {109, 127}, {110, 126}, {111, 126}, {111, 127}, {112, 127}, {113, 126}, {114, 126}, {114, 127}, {115, 127}, {116, 126}, {117, 126}, {117, 127}, {118, 127}, {119, 126}, {120, 126}, {120, 127}, {121, 127}, {122, 126}, {123, 126}, {123, 127}, {124, 127}, {125, 126}, {126, 126}, {126, 127}, {127, 127} };

		static constexpr uint32_t BC67_PARTITION_TABLE[2][64] =
		{
			{
				0x50505050, 0x40404040, 0x54545454, 0x54505040,
				0x50404000, 0x55545450, 0x55545040, 0x54504000,
				0x50400000, 0x55555450, 0x55544000, 0x54400000,
				0x55555440, 0x55550000, 0x55555500, 0x55000000,
				0x55150100, 0x00004054, 0x15010000, 0x00405054,
				0x00004050, 0x15050100, 0x05010000, 0x40505054,
				0x00404050, 0x05010100, 0x14141414, 0x05141450,
				0x01155440, 0x00555500, 0x15014054, 0x05414150,
				0x44444444, 0x55005500, 0x11441144, 0x05055050,
				0x05500550, 0x11114444, 0x41144114, 0x44111144,
				0x15055054, 0x01055040, 0x05041050, 0x05455150,
				0x14414114, 0x50050550, 0x41411414, 0x00141400,
				0x00041504, 0x00105410, 0x10541000, 0x04150400,
				0x50410514, 0x41051450, 0x05415014, 0x14054150,
				0x41050514, 0x41505014, 0x40011554, 0x54150140,
				0x50505500, 0x00555050, 0x15151010, 0x54540404
			},
			{
				0xAA685050, 0x6A5A5040, 0x5A5A4200, 0x5450A0A8,
				0xA5A50000, 0xA0A05050, 0x5555A0A0, 0x5A5A5050,
				0xAA550000, 0xAA555500, 0xAAAA5500, 0x90909090,
				0x94949494, 0xA4A4A4A4, 0xA9A59450, 0x2A0A4250,
				0xA5945040, 0x0A425054, 0xA5A5A500, 0x55A0A0A0,
				0xA8A85454, 0x6A6A4040, 0xA4A45000, 0x1A1A0500,
				0x0050A4A4, 0xAAA59090, 0x14696914, 0x69691400,
				0xA08585A0, 0xAA821414, 0x50A4A450, 0x6A5A0200,
				0xA9A58000, 0x5090A0A8, 0xA8A09050, 0x24242424,
				0x00AA5500, 0x24924924, 0x24499224, 0x50A50A50,
				0x500AA550, 0xAAAA4444, 0x66660000, 0xA5A0A5A0,
				0x50A050A0, 0x69286928, 0x44AAAA44, 0x66666600,
				0xAA444444, 0x54A854A8, 0x95809580, 0x96969600,
				0xA85454A8, 0x80959580, 0xAA141414, 0x96960000,
				0xAAAA1414, 0xA05050A0, 0xA0A5A5A0, 0x96000000,
				0x40804080, 0xA9A8A9A8, 0xAAAAAA44, 0x2A4A5254
			}
		};

		static constexpr Vec4ui32 ERROR_METRICS[] =
		{
			Vec4ui32(VECTOR_SET_ALL, 1),
			Vec4ui32(55, 75, 33, 100) // sqrt(0.3f, 0.56f, 0.11f) * 100
		};

		static constexpr std::tuple<uint32_t, uint32_t> BC67_INTERPOLATION_VALUES[4][16] =
		{
			{
				std::make_tuple<uint32_t, uint32_t>(64, 0), std::make_tuple<uint32_t, uint32_t>(33, 31), std::make_tuple<uint32_t, uint32_t>(0, 64), std::make_tuple<uint32_t, uint32_t>(0, 0),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0)
			},
			{
				std::make_tuple<uint32_t, uint32_t>(64, 0), std::make_tuple<uint32_t, uint32_t>(43, 21), std::make_tuple<uint32_t, uint32_t>(21, 43), std::make_tuple<uint32_t, uint32_t>(0, 64),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0)
			},
			{
				std::make_tuple<uint32_t, uint32_t>(64, 0), std::make_tuple<uint32_t, uint32_t>(55, 9), std::make_tuple<uint32_t, uint32_t>(46, 18), std::make_tuple<uint32_t, uint32_t>(37, 27),
				std::make_tuple<uint32_t, uint32_t>(27, 37), std::make_tuple<uint32_t, uint32_t>(18, 46), std::make_tuple<uint32_t, uint32_t>(9, 55), std::make_tuple<uint32_t, uint32_t>(0, 64),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0),
				std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0), std::make_tuple<uint32_t, uint32_t>(0, 0)
			},
			{
				std::make_tuple<uint32_t, uint32_t>(64, 0), std::make_tuple<uint32_t, uint32_t>(60, 4), std::make_tuple<uint32_t, uint32_t>(55, 9), std::make_tuple<uint32_t, uint32_t>(51, 13),
				std::make_tuple<uint32_t, uint32_t>(47, 17), std::make_tuple<uint32_t, uint32_t>(43, 21), std::make_tuple<uint32_t, uint32_t>(38, 26), std::make_tuple<uint32_t, uint32_t>(34, 30),
				std::make_tuple<uint32_t, uint32_t>(30, 34), std::make_tuple<uint32_t, uint32_t>(26, 38), std::make_tuple<uint32_t, uint32_t>(21, 43), std::make_tuple<uint32_t, uint32_t>(17, 47),
				std::make_tuple<uint32_t, uint32_t>(13, 51), std::make_tuple<uint32_t, uint32_t>(9, 55), std::make_tuple<uint32_t, uint32_t>(4, 60), std::make_tuple<uint32_t, uint32_t>(0, 64)
			}
		};

		static constexpr ModeInfo MODE_INFOS[] =
		{
			// Mode 0: Color only, 3 Subsets, RGBP 4441 (unique P-bit), 3-bit indecies, 16 partitions
			{ 3, 4, 6, 0, 0, 3, 0, Vec4ui8(4, 4, 4, 0), Vec4ui8(5, 5, 5, 0), PBitType::UNIQUE },
			// Mode 1: Color only, 2 Subsets, RGBP 6661 (shared P-bit), 3-bit indecies, 64 partitions
			{ 2, 6, 2, 0, 0, 3, 0, Vec4ui8(6, 6, 6, 0), Vec4ui8(7, 7, 7, 0), PBitType::SHARED },
			// Mode 2: Color only, 3 Subsets, RGB 555, 2-bit indecies, 64 partitions
			{ 3, 6, 0, 0, 0, 2, 0, Vec4ui8(5, 5, 5, 0), Vec4ui8(5, 5, 5, 0), PBitType::NONE },
			// Mode 3: Color only, 2 Subsets, RGBP 7771 (unique P-bit), 2-bits indecies, 64 partitions
			{ 2, 6, 4, 0, 0, 2, 0, Vec4ui8(7, 7, 7, 0), Vec4ui8(8, 8, 8, 0), PBitType::UNIQUE },
			// Mode 4: Color w/ Separate Alpha, 1 Subset, RGB 555, A6, 16x2/16x3-bit indices, 2-bit rotation, 1-bit index selector
			{ 1, 0, 0, 2, 1, 2, 3, Vec4ui8(5, 5, 5, 6), Vec4ui8(5, 5, 5, 6), PBitType::NONE },
			// Mode 5: Color w/ Separate Alpha, 1 Subset, RGB 777, A8, 16x2/16x2-bit indices, 2-bit rotation
			{ 1, 0, 0, 2, 0, 2, 2, Vec4ui8(7, 7, 7, 8), Vec4ui8(7, 7, 7, 8), PBitType::NONE },
			// Mode 6: Color+Alpha, 1 Subset, RGBAP 77771 (unique P-bit), 16x4-bit indecies
			{ 1, 0, 2, 0, 0, 4, 0, Vec4ui8(7, 7, 7, 7), Vec4ui8(8, 8, 8, 8), PBitType::UNIQUE },
			// Mode 7: Color+Alpha, 2 Subsets, RGBAP 55551 (unique P-bit), 2-bit indices, 64 partitions
			{ 2, 6, 4, 0, 0, 2, 0, Vec4ui8(5, 5, 5, 5), Vec4ui8(6, 6, 6, 6), PBitType::UNIQUE }
		};

		class Cluster {
		public:
			Cluster(const Vec4ui8* pixels, uint32_t num, const std::function<uint32_t(uint32_t, uint32_t, uint32_t)>& getPartition) :
				_getPartition(getPartition) {
				for (decltype(num) i = 0; i < num; ++i) {
					_dataPixels[i] = pixels[i];
					_dataPoints[i] = pixels[i];
				}
				_recalculate(false);
			}

			inline Vec4f32& SRK_CALL point(uint32_t index) {
				return _dataPoints[_pointMap[index]];
			}

			inline const Vec4f32& SRK_CALL point(uint32_t index) const {
				return _dataPoints[_pointMap[index]];
			}

			inline const Vec4ui8& SRK_CALL pixel(uint32_t index) const {
				return _dataPixels[_pointMap[index]];
			}

			inline uint32_t SRK_CALL numValidPoints() const {
				return _numValidPoints;
			}

			inline const Vec4f32& SRK_CALL avg() const {
				return _avg;
			}

			inline void SRK_CALL boundingBox(Vec4f32& minClr, Vec4f32& maxClr) const {
				minClr = _minClr;
				maxClr = _maxClr;
			}

			inline bool SRK_CALL allSamePoint() const {
				return _minClr == _maxClr;
			}

			uint32_t SRK_CALL principalAxis(Vec4f32& axis, float32_t* eigOne, float32_t* eigTwo) const {
				// We use these vectors for calculating the covariance matrix...
				std::array<Vec4f32, MAX_NUM_DATA_POINTS> toPts;
				Vec4f32 toPtsMax(-(std::numeric_limits<Vec4f32::ElementType>::max)(), -(std::numeric_limits<Vec4f32::ElementType>::max)(),
					-(std::numeric_limits<Vec4f32::ElementType>::max)(), -(std::numeric_limits<Vec4f32::ElementType>::max)());
				for (auto i = 0u; i < _numValidPoints; ++i) {
					toPts[i] = point(i) - avg();
					Math::max(toPtsMax.data, toPts[i].data, toPtsMax.data);
				}

				// Generate a list of unique points...
				std::array<Vec4f32, MAX_NUM_DATA_POINTS> upts;
				auto uptsIdx = 0u;
				for (auto i = 0u; i < _numValidPoints; ++i) {
					auto hasPt = false;
					for (auto j = 0u; j < uptsIdx; ++j) {
						if (upts[j] == point(i)) {
							hasPt = true;
							break;
						}
					}

					if (!hasPt) {
						upts[uptsIdx] = point(i);
						++uptsIdx;
					}
				}

				if (1 == uptsIdx) {
					axis = 0.f;
					return 0;
				} else {
					auto dir = upts[1] - upts[0];
					dir.normalize();
					bool collinear = true;
					for (auto i = 2u; i < _numValidPoints; ++i) {
						auto v = upts[i] - upts[0];
						if (!Math::equal(std::abs(Math::dot(v.data, dir.data)), v.getLength(), std::numeric_limits<float32_t>::epsilon())) {
							collinear = false;
							break;
						}
					}

					if (collinear) {
						axis = dir;
						return 0;
					}
				}

				Matrix4x4f32 covMat(nullptr);

				// Compute covariance.
				for (auto i = 0u; i < 4u; ++i) {
					for (auto j = 0u; j <= i; ++j) {
						auto sum = 0.f;
						for (auto k = 0u; k < _numValidPoints; ++k) {
							sum += toPts[k][i] * toPts[k][j];
						}

						covMat(i, j) = sum / 3.f;
						covMat(j, i) = covMat(i, j);
					}
				}

				auto iters = _powerMethod(covMat, axis, eigOne);
				if ((eigTwo != nullptr) && (eigOne != nullptr)) {
					if (*eigOne != 0) {
						Matrix4x4f32 reduced(nullptr);
						for (auto j = 0u; j < 4u; ++j) {
							for (auto i = 0u; i < 4u; ++i) {
								reduced(i, j) = axis[j] * axis[i];
							}
						}

						for (auto i = 0; i < 16; ++i) reduced[i] = covMat[i] - reduced[i] * (*eigOne);
						//reduced = cov_matrix - ((*eig_one) * reduced);
						auto allZero = true;
						for (auto i = 0u; i < 16u; ++i) {
							if (std::abs(reduced[i]) > 0.0005f) {
								allZero = false;
								break;
							}
						}

						if (allZero) {
							*eigTwo = 0;
						} else {
							Vec4f32 dummyDir(nullptr);
							iters += _powerMethod(reduced, dummyDir, eigTwo);
						}
					} else {
						*eigTwo = 0;
					}
				}

				return iters;
			}

			void SRK_CALL shapeIndex(uint32_t index, uint32_t numPartitions) {
				_shapeIndex = index;
				_numPartitions = numPartitions;
			}

			void SRK_CALL shapeIndex(uint32_t index) {
				_shapeIndex = index;
			}

			void SRK_CALL partition(uint32_t part) {
				_selectedPartition = part;
				_recalculate(true);
			}

			bool SRK_CALL isPointValid(uint32_t index) const {
				return _selectedPartition == _getPartition(_numPartitions, _shapeIndex, index);
			}

		private:
			uint32_t _numValidPoints;
			uint32_t _numPartitions;
			uint32_t _selectedPartition;
			uint32_t _shapeIndex;

			Vec4f32 _avg;

			std::array<Vec4f32, MAX_NUM_DATA_POINTS> _dataPoints;
			std::array<Vec4ui8, MAX_NUM_DATA_POINTS> _dataPixels;
			std::array<uint8_t, MAX_NUM_DATA_POINTS> _pointMap;
			Vec4f32 _minClr;
			Vec4f32 _maxClr;

			std::function<uint32_t(uint32_t, uint32_t, uint32_t)> _getPartition;

			void SRK_CALL _recalculate(bool considerValid) {
				_numValidPoints = 0u;
				_avg = Vec4f32(VECTOR_SET_ALL, 0);
				_minClr = Vec4f32(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
					std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
				_maxClr = Vec4f32(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
					-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

				auto map = 0u;
				for (size_t i = 0; i < _dataPoints.size(); ++i) {
					if (considerValid && !isPointValid(i)) continue;

					const auto& p = _dataPoints[i];

					++_numValidPoints;
					_avg += p;
					_pointMap[map] = (uint8_t)i;
					++map;

					Math::min(_minClr.data, p.data, _minClr.data);
					Math::max(_maxClr.data, p.data, _maxClr.data);
				}

				_avg /= (float32_t)_numValidPoints;
			}

			uint32_t SRK_CALL _powerMethod(const Matrix4x4f32& mat, Vec4f32& eigVec, float32_t* eigVal = nullptr) const {
				static auto const ITER_POWER = 4u;

				Vec4f32 b(VECTOR_SET_ALL, .5f);

				auto badEigenValue = false;
				auto fixed = false;
				auto numIterations = 1u;
				while (!fixed && (numIterations < ITER_POWER)) {
					Vec4f32 new_b(nullptr);
					mat.transformPoint<Math::Data2DDesc(Math::Hint::TRANSPOSE)>(b.data, new_b.data);

					auto new_b_len = new_b.getLength();
					if (new_b_len < 1e-6f) {
						if (badEigenValue) {
							eigVec = b;
							if (eigVal) *eigVal = 0;
							return numIterations;
						}

						b.set<false>(1, 1);

						b.normalize();
						badEigenValue = true;
						continue;
					}

					new_b.normalize();

					if (Math::equal(1.0f, Math::dot(b.data, new_b.data), std::numeric_limits<float32_t>::epsilon())) fixed = true;

					b = new_b;

					++numIterations;
				}

				eigVec = b;

				if (eigVal) {
					Vec4f32 result(nullptr);
					mat.transformPoint<Math::Data2DDesc(Math::Hint::TRANSPOSE)>(b.data, result.data);
					*eigVal = result.getLength() / b.getLength();
				}

				return numIterations;
			}
		};

		struct CompressParams {
			Vec4f32 p1[MAX_REGIONS], p2[MAX_REGIONS];
			uint8_t indices[MAX_REGIONS][16];
			uint8_t alphaIndices[16];
			uint8_t pbitCombo[MAX_REGIONS];
			int8_t rotationMode;
			int8_t indexMode;
			uint32_t shapeIndex;

			CompressParams() {}
			explicit CompressParams(uint32_t shape)
				: rotationMode(-1), indexMode(-1), shapeIndex(shape) {
				memset(p1, 0, sizeof(p1));
				memset(p2, 0, sizeof(p2));
				memset(indices, 0xFF, sizeof(indices));
				memset(alphaIndices, 0xFF, sizeof(alphaIndices));
				memset(pbitCombo, 0xFF, sizeof(pbitCombo));
			}
		};

		struct BlockConfig {
			ErrorMetric metric;
			int8_t rotateMode;
			int8_t indexMode;
			int32_t saSteps;
		};

		static void SRK_CALL _writeDdsHeader(uint8_t* buffer, const Config& cfg) {
			ByteArray ba(buffer, DDS_HEADER_SIZE);
			ba.write<uint32_t>(HEADER_MAGIC_ID);

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
				ba.write<uint32_t>((uint32_t)cfg.prefile);//dwCaps2 DXGI_FORMAT_BC7_UNORM
				ba.write<uint32_t>(3);//D3D10_RESOURCE_DIMENSION D3D10_RESOURCE_DIMENSION_TEXTURE2D
				ba.write<uint32_t>(0);//miscFlag
				ba.write<uint32_t>(1);//arraySize
				ba.write<uint32_t>(0);//miscFlags2
			}
		}

		static void SRK_CALL _encode(const Config& cfg, uint8_t* outBuffer, uint32_t threadIndex) {
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

				for (auto y = 0; y < 4; ++y) memcpy(in + (y << 4), cfg.data + (py + y) * bytesPerRow + px, 16);

				auto uniformBlock = true;
				for (auto i = 1; i < 16; ++i) {
					if (in32[i] != in32[0]) {
						uniformBlock = false;
						break;
					}
				}

				auto out = outBuffer + (startBlock << 4);

				if (uniformBlock) {
					_encodeUniformBlock(in, out);
				} else {
					Cluster blockCluster((Vec4ui8*)in, 16, [](uint32_t partitions, uint32_t shape, uint32_t offset) {
						return (partitions > 1) ? (BC67_PARTITION_TABLE[partitions - 2][shape] >> (offset * 2)) & 0x3 : 0;
						});

					auto metric = ErrorMetric::UNIFORM;
					int32_t saSteps;
					switch (cfg.quality) {
					case Quality::Best:
						saSteps = 50;
						break;
					case Quality::Balanced:
						saSteps = 10;
						break;
					default:
						saSteps = 0;
						break;
					}

					auto selection = _boxSelection(blockCluster, metric);
					//BOOST_ASSERT(selection.selectedModes != BlockMode::NONE);

					auto bestErr = std::numeric_limits<uint64_t>::max();
					auto bestMode = 8u;
					CompressParams bestParams;

					auto selectedModes = selection.selectedModes;
					auto numShapeIndices = selection.shapes.size();

					if (0 == numShapeIndices) {
						numShapeIndices = 1;
						selectedModes &= ~(BlockMode::TWO_PARTITION_MODES | BlockMode::THREE_PARTITION_MODES);
					}

					BlockConfig blkCfg;

					for (auto mode = 0u; mode < 8u; ++mode) {
						if ((selectedModes & (BlockMode)(1 << mode)) != BlockMode::NONE) {
							for (size_t shapeIndex = 0; shapeIndex < numShapeIndices; ++shapeIndex) {
								const auto& shape = selection.shapes[shapeIndex];

								uint32_t partitions = MODE_INFOS[mode].partitions;
								if ((1 == partitions) || (partitions == shape.numPartitions)) {
									if ((shape.index < 16) || (mode != 0)) {
										blockCluster.shapeIndex(shape.index, partitions);

										CompressParams params;
										uint64_t error = _tryCompress(blkCfg, mode, saSteps, metric, params, shape.index, blockCluster);
										if (error < bestErr) {
											bestErr = error;
											bestMode = mode;
											bestParams = params;
										}
									}
								}
							}
						}
					}
					//BOOST_ASSERT(bestMode < 8);
					blkCfg.indexMode = 0;
					_packBC7Block(blkCfg, bestMode, bestParams, out);
				}

				++startBlock;
			}
		}

		static uint32_t SRK_CALL _anchorIndexForSubset(uint32_t partition, uint32_t shapeIndex, uint32_t numPartitions) {
			static constexpr int32_t anchorIdx2[64] =
			{
				15, 15, 15, 15, 15, 15, 15, 15,
				15, 15, 15, 15, 15, 15, 15, 15,
				15, 2, 8, 2, 2, 8, 8, 15,
				2, 8, 2, 2, 8, 8, 2, 2,
				15, 15, 6, 8, 2, 8, 15, 15,
				2, 8, 2, 2, 2, 15, 15, 6,
				6, 2, 6, 8, 15, 15, 2, 2,
				15, 15, 15, 15, 15, 2, 2, 15
			};

			static constexpr int32_t anchorIdx3[2][64] =
			{
				{
					3, 3, 15, 15, 8, 3, 15, 15,
					8, 8, 6, 6, 6, 5, 3, 3,
					3, 3, 8, 15, 3, 3, 6, 10,
					5, 8, 8, 6, 8, 5, 15, 15,
					8, 15, 3, 5, 6, 10, 8, 15,
					15, 3, 15, 5, 15, 15, 15, 15,
					3, 15, 5, 5, 5, 8, 5, 10,
					5, 10, 8, 13, 15, 12, 3, 3
				},
				{
					15, 8, 8, 3, 15, 15, 3, 8,
					15, 15, 15, 15, 15, 15, 15, 8,
					15, 8, 15, 3, 15, 8, 15, 8,
					3, 15, 6, 10, 15, 15, 10, 8,
					15, 3, 15, 10, 10, 8, 9, 10,
					6, 15, 8, 15, 3, 6, 6, 8,
					15, 3, 15, 15, 15, 15, 15, 15,
					15, 15, 15, 15, 3, 15, 15, 8
				}
			};

			auto anchorIdx = 0;
			switch (partition) {
			case 1:
				anchorIdx = (2 == numPartitions) ? anchorIdx2[shapeIndex] : anchorIdx3[0][shapeIndex];
				break;

			case 2:
				//BOOST_ASSERT(3 == numPartitions);
				anchorIdx = anchorIdx3[1][shapeIndex];
				break;

			default:
				break;
			}

			return anchorIdx;
		}

		static void SRK_CALL _packBC7Block(const BlockConfig& cfg, int32_t mode, CompressParams& params, uint8_t* out) {
			const auto& modeInfo = MODE_INFOS[mode];
			const int32_t partition_bits = modeInfo.partitionBits;
			const int32_t partitions = modeInfo.partitions;

			size_t start_bit = 0;

			_writeBits(out, start_bit, mode + 1, 1 << mode);

			//BOOST_ASSERT(!partition_bits || ((((1 << partition_bits) - 1) & params.shape_index) == params.shape_index));
			_writeBits(out, start_bit, partition_bits, (uint8_t)params.shapeIndex);

			_writeBits(out, start_bit, modeInfo.rotationBits, params.rotationMode);
			_writeBits(out, start_bit, modeInfo.indexModeBits, params.indexMode);

			const auto qmask = _quantizationMask(modeInfo);

			Vec4ui8 pixel1[MAX_REGIONS];
			Vec4ui8 pixel2[MAX_REGIONS];
			for (auto i = 0; i < partitions; ++i) {
				switch (modeInfo.pBitType) {
				case PBitType::NONE:
					pixel1[i] = _quantize(params.p1[i], qmask);
					pixel2[i] = _quantize(params.p2[i], qmask);
					break;

				case PBitType::SHARED:
				case PBitType::UNIQUE:
					pixel1[i] = _quantize(params.p1[i], qmask, _pBitCombo(modeInfo, params.pbitCombo[i])[0]);
					pixel2[i] = _quantize(params.p2[i], qmask, _pBitCombo(modeInfo, params.pbitCombo[i])[1]);
					break;
				default:
					break;
					//KFL_UNREACHABLE("Invalid p-bit type");
				}
			}

			for (auto parIndex = 0; parIndex < partitions; ++parIndex) {
				int32_t anchorIndex = _anchorIndexForSubset(parIndex, params.shapeIndex, partitions);
				//BOOST_ASSERT(params.indices[parIndex][anchor_index] != 255);

				const auto alphaIndexBits = _numBitsPerAlpha(cfg, modeInfo, params.indexMode);
				const auto indexBits = _numBitsPerIndex(cfg, modeInfo, params.indexMode);
				if (params.indices[parIndex][anchorIndex] >> (indexBits - 1)) {
					std::swap(pixel1[parIndex], pixel2[parIndex]);

					int32_t indexVals = 1 << indexBits;
					for (auto i = 0; i < 16; ++i) params.indices[parIndex][i] = (uint8_t)((indexVals - 1) - params.indices[parIndex][i]);

					int32_t alpha_index_vals = 1 << alphaIndexBits;
					if (modeInfo.rotationBits != 0) {
						for (auto i = 0; i < 16; ++i) params.alphaIndices[i] = (uint8_t)((alpha_index_vals - 1) - params.alphaIndices[i]);
					}
				}

				const auto rotated = (params.alphaIndices[anchorIndex] >> (alphaIndexBits - 1)) > 0;
				if ((modeInfo.rotationBits != 0) && rotated) {
					std::swap(pixel1[parIndex][3], pixel2[parIndex][3]);

					int32_t alpha_index_vals = 1 << alphaIndexBits;
					for (auto i = 0; i < 16; ++i) params.alphaIndices[i] = (uint8_t)((alpha_index_vals - 1) - params.alphaIndices[i]);
				}

				//BOOST_ASSERT(!(params.indices[parIndex][anchorIndex] >> (indexBits - 1)));
				//BOOST_ASSERT((0 == modeInfo.rotation_bits)
				//	|| !(params.alpha_indices[anchorIndex] >> (alphaIndexBits - 1)));
			}

			for (auto i = 0; i < 4; ++i) {
				const int32_t bits = modeInfo.rgbaPrec[i];
				for (auto j = 0; j < partitions; ++j) {
					_writeBits(out, start_bit, bits, pixel1[j][i] >> (8 - bits));
					_writeBits(out, start_bit, bits, pixel2[j][i] >> (8 - bits));
				}
			}

			if (modeInfo.pBitType != PBitType::NONE) {
				for (auto s = 0; s < partitions; ++s) {
					const auto pbits = _pBitCombo(modeInfo, params.pbitCombo[s]);
					_writeBit(out, start_bit, (uint8_t)pbits[0]);
					if (modeInfo.pBitType != PBitType::SHARED) _writeBit(out, start_bit, (uint8_t)pbits[1]);
				}
			}

			if ((modeInfo.indexModeBits != 0) && (1 == params.indexMode)) {
				//BOOST_ASSERT(modeInfo.rotation_bits != 0);

				for (auto i = 0; i < 16; ++i) {
					const int32_t idx = params.alphaIndices[i];
					//BOOST_ASSERT(0 == AnchorIndexForSubset(0, params.shape_index, partitions));
					//BOOST_ASSERT(2 == this->NumBitsPerAlpha(modeInfo, params.index_mode));
					//BOOST_ASSERT((idx >= 0) && (idx < (1 << 2)));
					//BOOST_ASSERT_MSG((i != 0) || !(idx >> 1), "Leading bit of anchor index is not zero!");
					_writeBits(out, start_bit, (0 == i) ? 1 : 2, (uint8_t)idx);
				}

				for (auto i = 0; i < 16; ++i) {
					const int32_t idx = params.indices[0][i];
					//BOOST_ASSERT(0 == GetPartition(partitions, params.shape_index, i));
					//BOOST_ASSERT(0 == AnchorIndexForSubset(0, params.shape_index, partitions));
					//BOOST_ASSERT(this->NumBitsPerIndex(modeInfo, params.index_mode) == 3);
					//BOOST_ASSERT((idx >= 0) && (idx < (1 << 3)));
					//BOOST_ASSERT_MSG((i != 0) || !(idx >> 2), "Leading bit of anchor index is not zero!");
					_writeBits(out, start_bit, (0 == i) ? 2 : 3, (uint8_t)idx);
				}
			} else {
				for (auto i = 0; i < 16; ++i) {
					const int32_t subs = _getPartition(partitions, params.shapeIndex, i);
					const int32_t idx = params.indices[subs][i];
					const int32_t anchorIdx = _anchorIndexForSubset(subs, params.shapeIndex, partitions);
					const auto bitsForIdx = _numBitsPerIndex(cfg, modeInfo, params.indexMode);
					//BOOST_ASSERT((idx >= 0) && (idx < (1 << bits_for_idx)));
					//BOOST_ASSERT_MSG((i != anchor_idx) || !(idx >> (bits_for_idx - 1)),
					//	"Leading bit of anchor index is not zero!");
					_writeBits(out, start_bit, (i == anchorIdx) ? bitsForIdx - 1 : bitsForIdx, (uint8_t)idx);
				}

				if (modeInfo.rotationBits != 0) {
					for (auto i = 0; i < 16; ++i) {
						const int32_t idx = params.alphaIndices[i];
						const auto anchor_idx = 0;
						const auto bitsForIdx = _numBitsPerAlpha(cfg, modeInfo, params.indexMode);
						//BOOST_ASSERT((idx >= 0) && (idx < (1 << bits_for_idx)));
						//BOOST_ASSERT_MSG((i != anchor_idx) || (!(idx >> (bits_for_idx - 1))),
						//	"Leading bit of anchor index is not zero!");
						_writeBits(out, start_bit, (i == anchor_idx) ? bitsForIdx - 1 : bitsForIdx, (uint8_t)idx);
					}
				}
			}
		}

		static void SRK_CALL _encodeUniformBlock(uint8_t* in, uint8_t* out) {
			size_t startBit = 0;
			_writeBits(out, startBit, 6, 1 << 5);
			_writeBits(out, startBit, 2, 0);

			auto r = in[0];
			auto g = in[1];
			auto b = in[2];
			auto a = in[3];

			_writeBits(out, startBit, 7, O_MATCH7[r][0]);
			_writeBits(out, startBit, 7, O_MATCH7[r][1]);

			_writeBits(out, startBit, 7, O_MATCH7[g][0]);
			_writeBits(out, startBit, 7, O_MATCH7[g][1]);

			_writeBits(out, startBit, 7, O_MATCH7[b][0]);
			_writeBits(out, startBit, 7, O_MATCH7[b][1]);

			_writeBits(out, startBit, 8, a);
			_writeBits(out, startBit, 8, a);

			_writeBits(out, startBit, 8, 0xAB);
			_writeBits(out, startBit, 8, 0xAA);
			_writeBits(out, startBit, 8, 0xAA);
			_writeBits(out, startBit, 7, 0x2A);

			_writeBits(out, startBit, 8, 0xAB);
			_writeBits(out, startBit, 8, 0xAA);
			_writeBits(out, startBit, 8, 0xAA);
			_writeBits(out, startBit, 7, 0x2A);
		}

		static ShapeSelection SRK_CALL _boxSelection(Cluster& cluster, ErrorMetric metric) {
			ShapeSelection rst;

			auto opaque = true;
			for (auto i = 0; i < 16; ++i) {
				auto a = cluster.pixel(i)[3];
				opaque = opaque && (a >= 250);
			}

			auto bestErr = (std::numeric_limits<uint64_t>::max)();

			rst.shapes.resize(1);
			rst.shapes[0].numPartitions = 2;
			for (auto i = 0; i < 64; ++i) {
				cluster.shapeIndex(i, 2);

				uint64_t err = 0;
				for (auto ci = 0; ci < 2; ++ci) {
					cluster.partition(ci);
					err += _estimateNClusterError<8>(cluster, metric);
				}

				if (err < bestErr) {
					bestErr = err;
					rst.shapes[0].index = i;
				}

				// If it's small, we'll take it!
				if (err < 1) {
					rst.selectedModes = BlockMode::TWO_PARTITION_MODES;
					return rst;
				}
			}

			if (!opaque) {
				rst.selectedModes &= BlockMode::ALPHA_MODES;
				return rst;
			}

			rst.selectedModes &= ~(BlockMode::FOUR | BlockMode::FIVE);

			bestErr = std::numeric_limits<uint64_t>::max();

			rst.shapes.resize(2);
			rst.shapes[1].numPartitions = 3;
			for (auto i = 0; i < 64; ++i) {
				cluster.shapeIndex(i, 3);

				uint64_t err = 0;
				for (auto ci = 0; ci < 3; ++ci) {
					cluster.partition(ci);
					err += _estimateNClusterError<4>(cluster, metric);
				}

				if (err < bestErr) {
					bestErr = err;
					rst.shapes[1].index = i;
				}

				// If it's small, we'll take it!
				if (err < 1) {
					rst.selectedModes = BlockMode::THREE_PARTITION_MODES;
					return rst;
				}
			}

			return rst;
		}

		static void SRK_CALL _writeBit(uint8_t* out, size_t& start_bit, uint8_t val) {
			//BOOST_ASSERT((start_bit < 128) && (val < 2));

			size_t index = start_bit >> 3;
			size_t base = start_bit - (index << 3);
			out[index] &= ~(1 << base);
			out[index] |= val << base;
			++start_bit;
		}

		static void SRK_CALL _writeBits(uint8_t* out, size_t& startBit, size_t numBits, uint8_t val) {
			if (numBits == 0) return;

			size_t index = startBit >> 3;
			size_t base = startBit - (index << 3);
			if (base + numBits > 8) {
				size_t first_index_bits = 8 - base;
				size_t next_index_bits = numBits - first_index_bits;
				out[index + 0] &= ~(((1 << first_index_bits) - 1) << base);
				out[index + 0] |= val << base;
				out[index + 1] &= ~((1 << next_index_bits) - 1);
				out[index + 1] |= val >> first_index_bits;
			} else {
				out[index] &= ~(((1 << numBits) - 1) << base);
				out[index] |= val << base;
			}
			startBit += numBits;
		}

		template<size_t BUCKETS>
		static uint64_t SRK_CALL _estimateNClusterError(const Cluster& c, ErrorMetric metric) {
			Vec4f32 minClr(nullptr), maxClr(nullptr);
			c.boundingBox(minClr, maxClr);
			if (minClr.equal(maxClr, std::numeric_limits<float32_t>::epsilon())) return 0;

			const auto& w = ERROR_METRICS[(uint8_t)metric];
			return _quantizedError(c, minClr, maxClr, BUCKETS, Vec4ui8(VECTOR_SET_ALL, 255), w) * 2 + 1;
		}

		static uint64_t SRK_CALL _quantizedError(const Cluster& cluster, const Vec4f32& p1, const Vec4f32& p2,
			uint32_t buckets, const Vec4ui8& bitMask, const Vec4ui32& errorMetric,
			int32_t const pbits[2] = nullptr, uint8_t* indices = nullptr) {
			//BOOST_ASSERT((4 == buckets) || (8 == buckets) || (16 == buckets));

			uint32_t indexPrec;
			_bitScanForward(indexPrec, buckets);
			//BOOST_ASSERT((index_prec >= 2) && (index_prec <= 4));

			auto& interpVals = BC67_INTERPOLATION_VALUES[indexPrec - 1];
			
			Vec4ui8 qp1(nullptr), qp2(nullptr);
			if (pbits) {
				qp1 = _quantize(p1, bitMask, pbits[0]);
				qp2 = _quantize(p2, bitMask, pbits[1]);
			} else {
				qp1 = _quantize(p1, bitMask);
				qp2 = _quantize(p2, bitMask);
			}

			const Vec4f32 uqp1(qp1);
			const Vec4f32 uqp2(qp2);
			const auto uqpDir = uqp2 - uqp1;
			const auto uqplSq = (-uqpDir).getLengthSq();

			uint64_t totalErr = 0;
			if (0 == uqplSq) {
				for (auto i = 0u; i < cluster.numValidPoints(); ++i) {
					const auto& pixel = cluster.pixel(i);

					auto interp0 = std::get<0>(interpVals[0]);
					auto interp1 = std::get<1>(interpVals[0]);

					Vec4ui32 errVec(VECTOR_SET_ALL, 0);
					for (auto k = 0; k < 4; ++k) {
						int ip = (((qp1[k] * interp0) + (qp2[k] * interp1) + 32) >> 6) & 0xFF;
						int dist = abs(pixel[k] - ip);
						errVec[k] = dist * errorMetric[k];
					}

					totalErr += Math::dot(errVec.data, errVec.data);

					if (indices != nullptr) indices[i] = 0;
				}

				return totalErr;
			}

			for (auto i = 0u; i < cluster.numValidPoints(); ++i) {
				const auto& pt = cluster.point(i);
#if 0
				const auto pct = Math::clamp(Math::dot((pt - uqp1).data, uqpDir.data) / uqplSq, 0.0f, 1.0f) * (buckets - 1);
				const auto j1 = (int32_t)(pct);
				const auto j2 = (int32_t)(pct + 0.7f);
#else
				const auto pct = Math::dot((pt - uqp1).data, uqpDir.data) / uqplSq * (buckets - 1);
				const auto j1 = Math::clamp((int32_t)(std::floor(pct)), 0, (int32_t)(buckets - 1));
				const auto j2 = std::min((int32_t)(std::ceil(pct)), (int32_t)(buckets - 1));
#endif

				//BOOST_ASSERT((j1 >= 0) && (j2 <= static_cast<int32_t>(buckets - 1)));

				const auto& pixel = cluster.pixel(i);

				auto minErr = std::numeric_limits<uint64_t>::max();
				auto bestBucket = 0u;
				int32_t j = j1;
				do {
					auto interp0 = std::get<0>(interpVals[j]);
					auto interp1 = std::get<1>(interpVals[j]);

					Vec4ui32 errVec(VECTOR_SET_ALL, 0);
					for (auto k = 0; k < 4; ++k) {
						int ip = (((qp1[k] * interp0) + (qp2[k] * interp1) + 32) >> 6) & 0xFF;
						int dist = abs(pixel[k] - ip);
						errVec[k] = dist * errorMetric[k];
					}

					uint64_t error = Math::dot(errVec.data, errVec.data);
					if (error < minErr) {
						minErr = error;
						bestBucket = j;
					} else if (error > minErr + 1) {
						break;
					}

					++j;
				} while (j <= j2);

				totalErr += minErr;

				if (indices != nullptr) indices[i] = (uint8_t)bestBucket;
			}

			return totalErr;
		}

		static bool SRK_CALL _bitScanForward(uint32_t& index, uint32_t v) {
			if (0 == v) {
				index = 0;
				return false;
			} else {
				v &= ~v + 1;
				union {
					float32_t f;
					uint32_t u;
				} fnu;
				fnu.f = (float32_t)v;
				index = (fnu.u >> 23) - 127;
				return true;
			}
		}

		static Vec4ui8 SRK_CALL _quantize(const Vec4f32& p, const Vec4ui8& channelMask = Vec4ui8(VECTOR_SET_ALL, 255), int32_t bit = -1) {
			uint8_t const r = _quantizeChannel((uint32_t)(p[0]+ 0.5f) & 0xFF, channelMask[0], bit);
			uint8_t const g = _quantizeChannel((uint32_t)(p[1]+ 0.5f) & 0xFF, channelMask[1], bit);
			uint8_t const b = _quantizeChannel((uint32_t)(p[2]+ 0.5f) & 0xFF, channelMask[2], bit);
			uint8_t const a = _quantizeChannel((uint32_t)(p[3]+ 0.5f) & 0xFF, channelMask[3], bit);
			return Vec4ui8(r, g, b, a);
		}

		static uint8_t SRK_CALL _quantizeChannel(uint8_t val, uint8_t mask, int32_t bit = -1) {
			if (0xFF == mask) return val;
			if (0 == mask) return 0xFF;

			auto prec = _countBitsInMask(mask);
			const uint32_t step = 1 << (8 - prec);

			//BOOST_ASSERT(step - 1 == static_cast<uint8_t>(~mask));

			uint32_t lval = val & mask;
			uint32_t hval = lval + step;

			if (bit >= 0) {
				++prec;
				lval |= !!bit << (8 - prec);
				hval |= !!bit << (8 - prec);
			}

			if (lval > val) {
				lval -= step;
				hval -= step;
			}

			lval |= lval >> prec;
			hval |= hval >> prec;

			if (std::abs(val - (uint8_t)lval) < std::abs(val - (uint8_t)hval)) {
				return (uint8_t)lval;
			} else {
				return (uint8_t)hval;
			}
		}

		static uint32_t SRK_CALL _countBitsInMask(uint8_t n) {
			if (!n) return 0;

			uint32_t c;
			for (c = 0; n; ++c) n &= n - 1;
			return c;
		}

		static uint32_t SRK_CALL _getPartition(uint32_t partitions, uint32_t shape, uint32_t offset) {
			//BOOST_ASSERT((partitions <= 3) && (shape < 64) && (offset < 16));
			return (partitions > 1) ? (BC67_PARTITION_TABLE[partitions - 2][shape] >> (offset * 2)) & 0x3 : 0;
		}

		static uint64_t SRK_CALL _tryCompress(BlockConfig& cfg, int32_t mode, int32_t simulatedAnnealingSteps, ErrorMetric metric, CompressParams& params, uint32_t shapeIndex, Cluster& cluster) {
			cfg.metric = metric;
			cfg.rotateMode = 0;
			cfg.indexMode = 0;
			cfg.saSteps = simulatedAnnealingSteps;

			const auto& modeInfo = MODE_INFOS[mode];
			const int32_t partitions = modeInfo.partitions;

			params = CompressParams(shapeIndex);

			uint64_t totalErr = 0;
			for (int cidx = 0; cidx < partitions; ++cidx) {
				uint8_t indices[MAX_NUM_DATA_POINTS] = { 0 };
				cluster.partition(cidx);

				if (modeInfo.rotationBits != 0) {
					//BOOST_ASSERT(1 == partitions);

					uint8_t alphaIndices[MAX_NUM_DATA_POINTS];

					auto bestErr = std::numeric_limits<uint64_t>::max();
					for (auto rotMode = 0; rotMode < 4; ++rotMode) {
						cfg.rotateMode = rotMode;

						const auto idxModes = (4 == mode) ? 2 : 1;
						for (auto idxMode = 0; idxMode < idxModes; ++idxMode) {
							cfg.indexMode = idxMode;

							Vec4f32 v1(nullptr), v2(nullptr);
							auto error = _compressCluster(cfg, mode, cluster, v1, v2, indices, alphaIndices);

							if (error < bestErr) {
								bestErr = error;

								memcpy(params.indices[cidx], indices, sizeof(indices));
								memcpy(params.alphaIndices, alphaIndices, sizeof(alphaIndices));

								params.rotationMode = (int8_t)rotMode;
								params.indexMode = (int8_t)idxMode;

								params.p1[cidx] = v1;
								params.p2[cidx] = v2;
							}
						}
					}

					totalErr += bestErr;
				} else {
					totalErr += _compressCluster(cfg, mode, cluster, params.p1[cidx], params.p2[cidx], indices, params.pbitCombo[cidx]);

					int idx = 0;
					for (int i = 0; i < 16; ++i) {
						int subs = _getPartition(modeInfo.partitions, shapeIndex, i);
						if (subs == cidx) {
							params.indices[cidx][i] = indices[idx];
							++idx;
						}
					}
				}
			}

			return totalErr;
		}

		static int32_t SRK_CALL _numPbitCombos(const ModeInfo& modeInfo) {
			switch (modeInfo.pBitType) {
			case PBitType::NONE:
				return 1;
			case PBitType::SHARED:
				return 2;
			case PBitType::UNIQUE:
				return 4;
			default:
				return 1;
				//KFL_UNREACHABLE("Invalid p-bit type");
			}
		}

		static const int32_t* SRK_CALL _pBitCombo(const ModeInfo& modeInfo, int32_t idx) {
			static constexpr int32_t pbits[4][2] =
			{
				{ 0, 0 },
				{ 0, 1 },
				{ 1, 0 },
				{ 1, 1 }
			};

			switch (modeInfo.pBitType) {
			case PBitType::NONE:
				return pbits[0];
			case PBitType::SHARED:
				return idx ? pbits[3] : pbits[0];
			case PBitType::UNIQUE:
				return pbits[idx % 4];
			default:
				return pbits[0];
				//KFL_UNREACHABLE("Invalid p-bit type");
			}
		}

		static int32_t SRK_CALL _numBitsPerIndex(const BlockConfig& cfg, const ModeInfo& modeInfo, int8_t indexMode = -1) {
			if (indexMode < 0) indexMode = (uint8_t)cfg.indexMode;

			if (0 == indexMode) {
				return modeInfo.indexPrec1;
			} else {
				return modeInfo.indexPrec2;
			}
		}

		static int32_t SRK_CALL _numBitsPerAlpha(const BlockConfig& cfg, const ModeInfo& modeInfo, int8_t indexMode = -1) {
			if (indexMode < 0) indexMode = (uint8_t)cfg.indexMode;

			if (0 == indexMode) {
				return modeInfo.indexPrec2;
			} else {
				return modeInfo.indexPrec1;
			}
		}

		static uint64_t SRK_CALL _compressSingleColor(const BlockConfig& cfg, const ModeInfo& modeInfo, const Vec4ui8& pixel, Vec4f32& p1, Vec4f32& p2, uint8_t& bestPbitCombo) {
			auto bestErr = std::numeric_limits<uint64_t>::max();

			for (auto pbi = 0, numPbitCombos = _numPbitCombos(modeInfo); pbi < numPbitCombos; ++pbi) {
				const auto pbitCombo = _pBitCombo(modeInfo, pbi);

				Vec4ui32 dist(VECTOR_SET_ALL, 0);
				Vec4ui32 bestValI(VECTOR_SET_ALL, 0xFFFFFFFF);
				Vec4ui32 bestValJ(bestValI);

				for (size_t ci = 0; ci < dist.getDimension(); ++ci) {
					const uint8_t val = pixel[ci];
					auto bits = modeInfo.rgbaPrec[ci];

					if (0 == bits) {
						bestValI[ci] = bestValJ[ci] = 0xFF;
						dist[ci] = std::max(dist[ci], (uint32_t)(0xFF - val));
						continue;
					}

					const int32_t possVals = (1 << bits);
					int32_t possValsH[256];
					int32_t possValsL[256];

					const auto havePbit = (modeInfo.pBitType != PBitType::NONE);
					if (havePbit) ++bits;

					for (auto i = 0; i < possVals; ++i) {
						auto vh = i, vl = i;
						if (havePbit) {
							vh <<= 1;
							vl <<= 1;

							vh |= pbitCombo[1];
							vl |= pbitCombo[0];
						}

						possValsH[i] = (vh << (8 - bits));
						possValsH[i] |= (possValsH[i] >> bits);

						possValsL[i] = (vl << (8 - bits));
						possValsL[i] |= (possValsL[i] >> bits);
					}

					const auto bpi = (uint8_t)(_numBitsPerIndex(cfg, modeInfo) - 1);
					const auto interpVal0 = std::get<0>(BC67_INTERPOLATION_VALUES[bpi][1]);
					const auto interpVal1 = std::get<1>(BC67_INTERPOLATION_VALUES[bpi][1]);

					// Find the closest interpolated val that to the given val...
					auto bestChannelDist = 0xFFu;
					for (auto i = 0; (bestChannelDist > 0) && (i < possVals); ++i) {
						for (auto j = 0; (bestChannelDist > 0) && (j < possVals); ++j) {
							const uint32_t v1 = possValsL[i];
							const uint32_t v2 = possValsH[j];

							const uint32_t combo = (interpVal0 * v1 + interpVal1 * v2 + 32) >> 6;
							const uint32_t err = (combo > val) ? combo - val : val - combo;

							if (err < bestChannelDist) {
								bestChannelDist = err;
								bestValI[ci] = v1;
								bestValJ[ci] = v2;
							}
						}
					}

					dist[ci] = std::max(bestChannelDist, dist[ci]);
				}

				const auto& errWeights = ERROR_METRICS[(uint8_t)cfg.metric];
				uint64_t err = 0;
				for (size_t i = 0; i < dist.getDimension(); ++i) {
					uint32_t e = dist[i] * errWeights[i];
					err += e * e;
				}

				if (err < bestErr) {
					bestErr = err;
					bestPbitCombo = (uint8_t)pbi;

					for (size_t ci = 0; ci < p1.getDimension(); ++ci) {
						p1[ci] = (float32_t)bestValI[ci];
						p2[ci] = (float32_t)bestValJ[ci];
					}
				}
			}

			return bestErr;
		}

		static Vec4ui8 SRK_CALL _quantizationMask(const ModeInfo& modeInfo) {
			const int32_t mask_seed = 0xFFFFFF80;
			const uint32_t alpha_prec = modeInfo.rgbaPrec[3];
			const uint32_t cbits = modeInfo.rgbaPrec[0] - 1;
			const uint32_t abits = modeInfo.rgbaPrec[3] - 1;
			return Vec4ui8(alpha_prec > 0 ? (mask_seed >> abits) & 0xFF : 0, (mask_seed >> cbits) & 0xFF, (mask_seed >> cbits) & 0xFF, (mask_seed >> cbits) & 0xFF);
		}

		static void SRK_CALL _clampEndpoints(Vec4f32& p1, Vec4f32& p2) {
			Math::clamp<Math::Hint::NONE>(p1.data, 0.0f, 255.0f, p1.data);
			Math::clamp<Math::Hint::NONE>(p2.data, 0.0f, 255.0f, p2.data);
		}

		static void SRK_CALL _clampEndpointsToGrid(const ModeInfo& modeInfo, Vec4f32& p1, Vec4f32& p2, uint8_t& bestPbitCombo) {
			const auto pbitCombos = _numPbitCombos(modeInfo);
			const auto hasPbits = pbitCombos > 1;
			const auto qmask = _quantizationMask(modeInfo);

			_clampEndpoints(p1, p2);

			auto minDist = std::numeric_limits<float32_t>::max();
			Vec4f32 bp1, bp2;
			for (auto i = 0; i < pbitCombos; ++i) {
				Vec4f32 np1(nullptr), np2(nullptr);
				if (hasPbits) {
					np1 = _quantize(p1, qmask, _pBitCombo(modeInfo, i)[0]);
					np2 = _quantize(p2, qmask, _pBitCombo(modeInfo, i)[1]);
				} else {
					np1 = _quantize(p1, qmask);
					np2 = _quantize(p2, qmask);
				}

				auto d1 = np1 - p1;
				auto d2 = np2 - p2;
				auto dist = d1.getLengthSq() + d2.getLengthSq();
				if (dist < minDist) {
					minDist = dist;
					bp1 = np1;
					bp2 = np2;
					bestPbitCombo = (uint8_t)i;
				}
			}

			p1 = bp1;
			p2 = bp2;
		}

		template<typename T>
		static void SRK_CALL _rotation(Vec4<T>& v, int mode) {
			switch (mode) {
			case 0:
				break;
			case 1:
				std::swap(v[0], v[3]);
				break;
			case 2:
				std::swap(v[1], v[3]);
				break;
			case 3:
				std::swap(v[2], v[3]);
				break;
			default:
				break;
				//KFL_UNREACHABLE("Invalid rotation mode");
			}
		}

		static Vec4ui32 SRK_CALL _errorMetric(const BlockConfig& cfg, const ModeInfo& modeInfo) {
			auto w = ERROR_METRICS[(uint8_t)cfg.metric];
			_rotation(w, _rotationMode(cfg, modeInfo));
			return w;
		}

		static int32_t SRK_CALL _intRand() {
			static thread_local std::mt19937 gen;
			std::uniform_int_distribution<int32_t> random_dis(0, RAND_MAX);
			return random_dis(gen);
		}

		static void SRK_CALL _changePointForDirWithPbitChange(Vec4f32& v, uint32_t dir, uint32_t oldPbit, const Vec4f32& step) {
			if ((dir & 1UL) && (0 == oldPbit)) {
				v[0] -= step[0];
			} else if (!(dir & 1UL) && (1 == oldPbit)) {
				v[0] += step[0];
			}

			if ((dir & 2UL) && (0 == oldPbit)) {
				v[1] -= step[1];
			} else if (!(dir & 2UL) && (1 == oldPbit)) {
				v[1] += step[1];
			}

			if ((dir & 4UL) && (0 == oldPbit)) {
				v[2] -= step[2];
			} else if (!(dir & 4UL) && (1 == oldPbit)) {
				v[2] += step[2];
			}

			if ((dir & 8UL) && (0 == oldPbit)) {
				v[3] -= step[3];
			} else if (!(dir & 8UL) && (1 == oldPbit)) {
				v[3] += step[3];
			}
		}

		static void SRK_CALL _changePointForDirWithoutPbitChange(Vec4f32& v, uint32_t dir, const Vec4f32& step) {
			if (dir & 1UL) {
				v[0] -= step[0];
			} else {
				v[0] += step[0];
			}

			if (dir & 2UL) {
				v[1] -= step[1];
			} else {
				v[1] += step[1];
			}

			if (dir & 4UL) {
				v[2] -= step[2];
			} else {
				v[2] += step[2];
			}

			if (dir & 8UL) {
				v[3] -= step[3];
			} else {
				v[3] += step[3];
			}
		}

		static void SRK_CALL _pickBestNeighboringEndpoints(const BlockConfig& cfg, int32_t mode, const Vec4f32& p1, const Vec4f32& p2, int32_t curPbitCombo, Vec4f32& np1, Vec4f32& np2, int32_t& pbitCombo, float32_t stepSz = 1.f) {
			const auto& modeInfo = MODE_INFOS[mode];

			Vec4f32 step((float32_t)(1 << (8 - modeInfo.rgbaPrec[0])), (float32_t)(1 << (8 - modeInfo.rgbaPrec[1])), (float32_t)(1 << (8 - modeInfo.rgbaPrec[2])), (float32_t)(1 << (8 - modeInfo.rgbaPrec[3])));
			step *= stepSz;
			if (mode < 4) step[(_rotationMode(cfg, modeInfo) + 3) & 0x3] = 0;

			const auto hasPbits = modeInfo.pBitType != PBitType::NONE;
			if (hasPbits) {
				if (PBitType::SHARED == modeInfo.pBitType) {
					pbitCombo = (curPbitCombo + 1) % 2;
				} else {
					pbitCombo = 3 - curPbitCombo;
				}

				//BOOST_ASSERT(1 == this->PBitCombo(modeInfo, curPbitCombo)[0]
				//	+ this->PBitCombo(modeInfo, pbitCombo)[0]);
				//BOOST_ASSERT(1 == this->PBitCombo(modeInfo, curPbitCombo)[1]
				//	+ this->PBitCombo(modeInfo, pbitCombo)[1]);
			}

			for (auto pt = 0; pt < 2; ++pt) {
				const auto& p = pt ? p1 : p2;
				auto& np = pt ? np1 : np2;
				const uint32_t rdir = _intRand() & 0xF;

				np = p;
				if (hasPbits) {
					const uint32_t pbit = _pBitCombo(modeInfo, curPbitCombo)[pt];
					_changePointForDirWithPbitChange(np, rdir, pbit, step);
				} else {
					_changePointForDirWithoutPbitChange(np, rdir, step);
				}

				Math::clamp<Math::Hint::NONE>(np.data, 0.f, 255.f, np.data);
			}
		}

		static bool SRK_CALL _acceptNewEndpointError(uint64_t newErr, uint64_t oldErr, float32_t temp) {
			if (newErr < oldErr)return true;

			const auto p = (size_t)(exp(0.1f * (int64_t)(oldErr - newErr) / temp) * (float32_t)(RAND_MAX));
			const size_t r = _intRand();

			return r < p;
		}

		static uint64_t SRK_CALL _optimizeEndpointsForCluster(const BlockConfig& cfg, int32_t mode, const Cluster& cluster, Vec4f32& p1, Vec4f32& p2, uint8_t* bestIndices, uint8_t& bestPbitCombo) {
			const auto& modeInfo = MODE_INFOS[mode];
			const uint32_t buckets = (1 << _numBitsPerIndex(cfg, modeInfo));
			const auto qmask = _quantizationMask(modeInfo);

			auto curErr = _quantizedError(cluster, p1, p2, buckets, qmask, _errorMetric(cfg, modeInfo), _pBitCombo(modeInfo, bestPbitCombo), bestIndices);

			int32_t curPbitCombo = bestPbitCombo;
			auto bestErr = curErr;

			if (modeInfo.pBitType != PBitType::NONE) {
				p1 = _quantize(p1, qmask, _pBitCombo(modeInfo, bestPbitCombo)[0]);
				p2 = _quantize(p2, qmask, _pBitCombo(modeInfo, bestPbitCombo)[1]);
			} else {
				p1 = _quantize(p1, qmask);
				p2 = _quantize(p2, qmask);
			}

			auto bp1 = p1, bp2 = p2;

			const auto maxEnergy = cfg.saSteps;
			for (auto energy = 0; (bestErr > 0) && (energy < maxEnergy); ++energy) {
				auto temp = (energy + 0.5f) / (float32_t)(maxEnergy - 0.5f);

				uint8_t indices[MAX_NUM_DATA_POINTS];
				Vec4f32 np1, np2;
				auto pbitCombo = 0;

				_pickBestNeighboringEndpoints(cfg, mode, p1, p2, curPbitCombo, np1, np2, pbitCombo);

				auto error = _quantizedError(cluster, np1, np2, buckets, qmask, _errorMetric(cfg, modeInfo), _pBitCombo(modeInfo, pbitCombo), indices);

				if (_acceptNewEndpointError(error, curErr, temp)) {
					curErr = error;
					p1 = np1;
					p2 = np2;
					curPbitCombo = pbitCombo;
				}

				if (error < bestErr) {
					memcpy(bestIndices, indices, sizeof(indices));
					bp1 = np1;
					bp2 = np2;
					bestPbitCombo = (uint8_t)pbitCombo;
					bestErr = error;

					energy = 0;
				}
			}

			p1 = bp1;
			p2 = bp2;

			return bestErr;
		}

		static uint64_t SRK_CALL _compressCluster(const BlockConfig& cfg, int32_t mode, const Cluster& cluster, Vec4f32& p1, Vec4f32& p2, uint8_t* bestIndices, uint8_t& bestPbitCombo) {
			const auto& modeInfo = MODE_INFOS[mode];

			if (cluster.allSamePoint()) {
				const auto& p = cluster.pixel(0);
				auto bestErr = _compressSingleColor(cfg, modeInfo, p, p1, p2, bestPbitCombo);

				for (auto i = 0u; i < cluster.numValidPoints(); ++i) bestIndices[i] = 1;
				return cluster.numValidPoints() * bestErr;
			}

			const uint32_t buckets = (1 << _numBitsPerIndex(cfg, modeInfo));

#if 1
			Vec4f32 axis(nullptr);
			cluster.principalAxis(axis, nullptr, nullptr);

			auto minDp = std::numeric_limits<float32_t>::max();
			auto maxDp = -std::numeric_limits<float32_t>::max();
			for (auto i = 0u; i < cluster.numValidPoints(); ++i) {
				auto dp = Math::dot((cluster.point(i) - cluster.avg()).data, axis.data);
				if (dp < minDp) {
					minDp = dp;
				}
				if (dp > maxDp) {
					maxDp = dp;
				}
			}

			p1 = cluster.avg() + minDp * axis;
			p2 = cluster.avg() + maxDp * axis;
#else
			cluster.boundingBox(p1, p2);
#endif

			_clampEndpoints(p1, p2);

			Vec4f32 pts[1 << 4];
			uint32_t numPts[1 << 4];
			//BOOST_ASSERT(buckets <= 1 << 4);

			for (auto i = 0u; i < buckets; ++i) {
				auto s = i / (float32_t)(buckets - 1);
				Math::lerp<Math::Hint::NONE>(p1.data, p2.data, s, pts[i].data);
			}

			uint32_t bucketIdx[MAX_NUM_DATA_POINTS] = { 0 };

			auto fixed = false;
			while (!fixed) {
				Vec4f32 newPts[1 << 4];

				for (auto i = 0u; i < cluster.numValidPoints(); ++i) {
					auto minBucket = -1;
					auto minDist = std::numeric_limits<float32_t>::max();

					for (auto j = 0u; j < buckets; ++j) {
						auto v = cluster.point(i) - pts[j];
						auto distSq = v.getLengthSq();
						if (distSq < minDist) {
							minDist = distSq;
							minBucket = j;
						}
					}

					//BOOST_ASSERT(min_bucket >= 0);
					bucketIdx[i] = minBucket;
				}

				for (auto i = 0u; i < buckets; ++i) {
					numPts[i] = 0;
					newPts[i] = 0;
					for (auto j = 0u; j < cluster.numValidPoints(); ++j) {
						if (bucketIdx[j] == i) {
							++numPts[i];
							newPts[i] += cluster.point(j);
						}
					}

					if (0 != numPts[i]) newPts[i] /= (float32_t)numPts[i];
				}

				fixed = true;
				for (auto i = 0u; i < buckets; ++i) {
					if (!pts[i].equal(newPts[i], std::numeric_limits<float32_t>::epsilon())) {
						fixed = false;
						break;
					}
				}

				for (auto i = 0u; i < buckets; ++i) pts[i] = newPts[i];
			}

			auto numBucketsFilled = 0;
			auto lastFilledBucket = -1;
			for (auto i = 0u; i < buckets; ++i) {
				if (numPts[i] > 0) {
					++numBucketsFilled;
					lastFilledBucket = i;
				}
			}

			//BOOST_ASSERT(num_buckets_filled > 0);
			if (1 == numBucketsFilled) {
				const auto p = _quantize(pts[lastFilledBucket]);
				auto bestErr = _compressSingleColor(cfg, modeInfo, p, p1, p2, bestPbitCombo);

				for (auto i = 0u; i < cluster.numValidPoints(); ++i) bestIndices[i] = 1;
				return cluster.numValidPoints() * bestErr;
			}

			float32_t asq = 0, bsq = 0, ab = 0;
			Vec4f32 ax(VECTOR_SET_ALL, 0), bx(VECTOR_SET_ALL, 0);
			for (auto i = 0u; i < buckets; ++i) {
				const auto& x = pts[i];
				const auto n = numPts[i];

				const auto fbi = (float32_t)(buckets - 1 - i);
				const auto fb = (float32_t)(buckets - 1);
				const auto fi = (float32_t)i;
				const auto fn = (float32_t)n;

				const auto a = fbi / fb;
				const auto b = fi / fb;

				asq += fn * a * a;
				bsq += fn * b * b;
				ab += fn * a * b;

				ax += x * a * fn;
				bx += x * b * fn;
			}

			auto f = 1 / (asq * bsq - ab * ab);
			p1 = f * (ax * bsq - bx * ab);
			p2 = f * (bx * asq - ax * ab);

			_clampEndpointsToGrid(modeInfo, p1, p2, bestPbitCombo);

			return _optimizeEndpointsForCluster(cfg, mode, cluster, p1, p2, bestIndices, bestPbitCombo);
		}

		template <int32_t N>
		inline static uint8_t SRK_CALL _extendNTo8Bits(int32_t input) {
			return (uint8_t)((input >> (N - (8 - N))) | (input << (8 - N)));
		}

		static uint64_t SRK_CALL _compressCluster(const BlockConfig& cfg, int32_t mode, const Cluster& cluster, Vec4f32& p1, Vec4f32& p2, uint8_t* best_indices, uint8_t* alpha_indices) {
			//BOOST_ASSERT((4 == mode) || (5 == mode));
			//BOOST_ASSERT(1 == mode_info_[mode].partitions);
			//BOOST_ASSERT(BC67_MAX_NUM_DATA_POINTS == cluster.NumValidPoints());
			//BOOST_ASSERT(mode_info_[mode].rgba_prec.a() > 0);

			//BOOST_ASSERT_MSG(!cluster.AllSamePoint(), "We should only be using this function in modes 4 & 5 that have a"
			//	"single subset, in which case single colors should have been"
			//	"detected much earlier.");

			const auto& modeInfo = MODE_INFOS[mode];

			Cluster rgbCluster(cluster);
			float32_t alphaVals[MAX_NUM_DATA_POINTS] = { 0 };

			auto alphaMin = std::numeric_limits<float32_t>::max();
			auto alphaMax = -std::numeric_limits<float32_t>::max();
			for (auto i = 0u; i < rgbCluster.numValidPoints(); ++i) {
				auto& v = rgbCluster.point(i);
				_rotation(v, _rotationMode(cfg, modeInfo));

				alphaVals[i] = v[3];
				v[3] = 255.0f;

				alphaMin = std::min(alphaMin, alphaVals[i]);
				alphaMax = std::max(alphaMax, alphaVals[i]);
			}

			uint8_t dummyPbit = 0;
			Vec4f32 rgb_p1(nullptr), rgb_p2(nullptr);
			auto rgb_err = _compressCluster(cfg, mode, rgbCluster, rgb_p1, rgb_p2, best_indices, dummyPbit);

			auto a1 = alphaMin;
			auto a2 = alphaMax;
			auto alphaErr = std::numeric_limits<uint64_t>::max();

			const auto& interpVals = BC67_INTERPOLATION_VALUES[_numBitsPerAlpha(cfg, modeInfo) - 1];

			const auto weight = _errorMetric(cfg, modeInfo)[3];

			const uint32_t numBuckets = (1 << _numBitsPerAlpha(cfg, modeInfo));

			if (a1 == a2) {
				uint8_t const a_be = static_cast<uint8_t>(a1);

				if (5 == mode) {
					for (auto i = 0u; i < MAX_NUM_DATA_POINTS; ++i) {
						alpha_indices[i] = 0;
					}

					alphaErr = 0;
				} else {
					//BOOST_ASSERT(4 == mode);

					uint32_t a1i = _extendNTo8Bits<6>(O_MATCH6[a_be][1]);
					uint32_t a2i = _extendNTo8Bits<6>(O_MATCH6[a_be][0]);

					if (1 == cfg.indexMode) {
						for (auto i = 0u; i < MAX_NUM_DATA_POINTS; ++i) alpha_indices[i] = 1;
					} else {
						for (auto i = 0u; i < MAX_NUM_DATA_POINTS; ++i) alpha_indices[i] = 2;
					}

					auto interp0 = std::get<0>(interpVals[alpha_indices[0] & 0xFF]);
					auto interp1 = std::get<1>(interpVals[alpha_indices[0] & 0xFF]);

					const uint8_t ip = (((a1i * interp0) + (a2i * interp1) + 32) >> 6) & 0xFF;
					uint64_t pxErr = weight * abs(a_be - ip);
					pxErr *= pxErr;
					alphaErr = 16 * pxErr;

					a1 = (float32_t)a1i;
					a2 = (float32_t)a2i;
				}
			} else {
				float32_t vals[1 << 3];
				memset(vals, 0, sizeof(vals));

				uint32_t buckets[MAX_NUM_DATA_POINTS];

				for (auto i = 0u; i < numBuckets; ++i) {
					float const fi = static_cast<float>(i);
					float const fb = static_cast<float>(numBuckets - 1);
					vals[i] = alphaMin + (fi / fb) * (alphaMax - alphaMin);
				}

				// Assign each value to a bucket
				for (auto i = 0u; i < MAX_NUM_DATA_POINTS; ++i) {
					auto minDist = 255.f;
					buckets[i] = numBuckets;
					for (auto j = 0u; j < numBuckets; ++j) {
						auto dist = std::abs(alphaVals[i] - vals[j]);
						if (dist < minDist) {
							minDist = dist;
							buckets[i] = j;
						}
					}
				}

				float32_t npts[1 << 3];

				auto fixed = false;
				while (!fixed) {
					memset(npts, 0, sizeof(npts));

					float32_t avg[1 << 3];
					memset(avg, 0, sizeof(avg));

					// Calculate average of each cluster
					for (auto i = 0u; i < numBuckets; ++i) {
						for (auto j = 0u; j < MAX_NUM_DATA_POINTS; ++j) {
							if (buckets[j] == i) {
								avg[i] += alphaVals[j];
								npts[i] += 1.f;
							}
						}

						if (npts[i] > 0.f) avg[i] /= npts[i];
					}

					fixed = true;
					for (auto i = 0u; i < numBuckets; ++i) {
						fixed = fixed && (avg[i] == vals[i]);
						if (!fixed) break;
					}

					memcpy(vals, avg, sizeof(vals));

					for (auto i = 0u; i < MAX_NUM_DATA_POINTS; ++i) {
						auto minDist = 255.f;
						for (auto j = 0u; j < numBuckets; ++j) {
							float dist = std::abs(alphaVals[i] - vals[j]);
							if (dist < minDist) {
								minDist = dist;
								buckets[i] = j;
							}
						}
					}
				}

				float32_t asq = 0, bsq = 0, ab = 0;
				float32_t ax = 0, bx = 0;
				for (auto i = 0u; i < numBuckets; ++i) {
					const auto fbi = (float32_t)(numBuckets - 1 - i);
					const auto fb = (float32_t)(numBuckets - 1);
					const auto fi = (float32_t)i;

					auto a = fbi / fb;
					auto b = fi / fb;

					auto n = npts[i];
					auto x = vals[i];

					asq += n * a * a;
					bsq += n * b * b;
					ab += n * a * b;

					ax += x * a * n;
					bx += x * b * n;
				}

				auto f = 1.0f / (asq * bsq - ab * ab);
				a1 = f * (ax * bsq - bx * ab);
				a2 = f * (bx * asq - ax * ab);

				a1 = Math::clamp(a1, 0.0f, 255.0f);
				a2 = Math::clamp(a2, 0.0f, 255.0f);

				const int8_t maskSeed = -0x7F;
				const auto a1b = _quantizeChannel((uint8_t)a1, (maskSeed >> (modeInfo.rgbaPrec[3] - 1)));
				const auto a2b = _quantizeChannel((uint8_t)a2, (maskSeed >> (modeInfo.rgbaPrec[3] - 1)));

				alphaErr = 0;
				for (auto i = 0u; i < MAX_NUM_DATA_POINTS; ++i) {
					auto val = (uint8_t)alphaVals[i];

					auto minErr = std::numeric_limits<uint64_t>::max();
					auto bestBucket = -1;

					for (auto j = 0u; j < numBuckets; ++j) {
						auto interp0 = std::get<0>(interpVals[j]);
						auto interp1 = std::get<1>(interpVals[j]);

						const uint8_t ip = (((a1b * interp0) + (a2b * interp1) + 32) >> 6) & 0xFF;
						uint64_t pxErr = weight * abs(val - ip);
						pxErr *= pxErr;

						if (pxErr < minErr) {
							minErr = pxErr;
							bestBucket = j;
						}
					}

					alphaErr += minErr;
					alpha_indices[i] =(uint8_t)bestBucket;
				}
			}

			for (size_t i = 0; i < p1.getDimension(); ++i) {
				p1[i] = (i == (p1.getDimension() - 1)) ? a1 : rgb_p1[i];
				p2[i] = (i == (p2.getDimension() - 1)) ? a2 : rgb_p2[i];
			}

			return rgb_err + alphaErr;
		}

		static int32_t SRK_CALL _rotationMode(const BlockConfig& cfg, const ModeInfo& modeInfo) {
			return modeInfo.rotationBits ? cfg.rotateMode : 0;
		}
	};
}

namespace srk::meta {
	template<int64_t I, int64_t N, auto... NonTypeArgs, typename Fn, typename... Args>
	concept aabbf = requires(Fn&& fn, Args&&... args) {
		fn.operator()<I, N, NonTypeArgs...>(std::forward<Args>(args)...);
		//std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
		/* not required to be equality preserving */
	};

	template <typename Tuple, std::size_t... I>
	void process(Tuple const& tuple, std::index_sequence<I...>) {
		printaln(std::get<I>(tuple)...);
	}

	template <typename Tuple>
	void process(Tuple const& tuple) {
		process(tuple, std::make_index_sequence<std::tuple_size<Tuple>::value>());
	}

	template<auto... NonTypeArgs, typename Fn, typename... Args>
	//requires (aabbf<I, N, NonTypeArgs..., Fn, Args...>)
	inline void loop(Fn&& fn, Args&&... args) {
		constexpr auto ret = fn.operator()<NonTypeArgs...>(std::forward<Args>(args)...);
		//if constexpr (ret) {
		//	loop<*ret>(std::forward<Fn>(fn), std::forward<Args>(args)...);
		//}
		//if constexpr (std::get<0>(ret)) {
			//process(nextNonTypeArgs);
			//constexpr auto i = std::make_index_sequence<std::tuple_size_v<nextNonTypeArgs>>;
			//loop<std::get<std::make_index_sequence<std::tuple_size<nextNonTypeArgs>::value>>(nextNonTypeArgs)...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
		//}
	}

	/*template<int64_t I, int64_t N, typename Fn, typename... Args>
	inline void loopIf(Fn&& fn, Args&&... args) {
		if constexpr (I < N) {
			if constexpr (fn.operator()(std::forward<Args>(args)...)) {
				loop<I + 1, N>(std::forward<Fn>(fn), std::forward<Args>(args)...);
			}
		}
	}*/

	/*template<int64_t I, typename Fn, typename... Args>
	inline void loop(Fn&& fn, Args&&... args) {
		if constexpr (fn.operator()(std::forward<Args>(args)...)) {
			loop<I + 1>(std::forward<Fn>(fn), std::forward<Args>(args)...);
		}
	}*/
}

class CompressTextureTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		auto srcDir = getAppPath().parent_path().u8string();
		auto src = extensions::PNGConverter::decode(readFile(srcDir + "/Resources/tex2.png"));
		if (!src) return 0;

		if (src->format == modules::graphics::TextureFormat::R8G8B8) {
			ByteArray dst(src->size.getMultiplies() * 4);
			dst.setLength(dst.getCapacity());
			Image::convertFormat(src->size, src->format, src->source.getSource(), modules::graphics::TextureFormat::R8G8B8A8, dst.getSource());
			src->format = modules::graphics::TextureFormat::R8G8B8A8;
			src->source = std::move(dst);
		}

		//meta::loop<1, 2>(lmd, 1, 2, 3);

		src->flipY();

		auto t0 = Time::now();
		//auto astc = extensions::ASTCConverter::encode(*src, Vec3ui32(4, 4, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Quality::MEDIUM, extensions::ASTCConverter::Flags::WRITE_HEADER, 12);
		//printaln(Time::now() - t0);
		//writeFile(srcDir + "/img.astc", astc);

		auto bc7 = extensions::BC7Converter::encode(*src, extensions::BC7Converter::Profile::UNORM, extensions::BC7Converter::Quality::Fast, extensions::BC7Converter::Flags::WRITE_DDS_HEADER, 12);
		printaln(Time::now() - t0);
		writeFile(srcDir + "/img.dds", bc7);

		return 0;
	}
};