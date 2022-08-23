#pragma once

#include "../BaseTester.h"
#include "srk/math/Math.h"

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
			Best,
			Balanced,
			Fast
		};

		enum class Flags : uint8_t {
			NONE = 0,
			WRITE_DDS_HEADER = 1 << 0
		};

		struct Config {
			Profile prefile;
			Vec2<size_t> size;
			Vec2<size_t> blocks;
			size_t numBlocks;
			size_t numBlocksPerThread;
			const uint8_t* data;
		};

		enum BlockMode : uint8_t {
			ZERO = 1 << 0,
			ONE = 1 << 1,
			TWO = 1 << 2,
			THREE = 1 << 3,
			FOUR = 1 << 4,
			FIVE = 1 << 5,
			SIX = 1 << 6,
			SEVEN = 1 << 7
		};

		struct Shape {
			uint32_t numPartitions;
			uint32_t index;
		};

		struct ShapeSelection {
			std::vector<Shape> shapes;
			uint32_t selected_modes;

			ShapeSelection()
				: selected_modes((BlockMode)(0xFF)) {}
		};

		class Cluster {
		public:
			Cluster(const Vec4ui8* pixels, uint32_t num, const std::function<uint32_t(uint32_t, uint32_t, uint32_t)>& getPartition) :
				get_partition_(getPartition) {
				for (decltype(num) i = 0; i < num; ++i) {
					data_pixels_[i] = pixels[i];
					data_points_[i] = pixels[i];
				}
				recalculate(false);
			}

			inline Vec4f32& point(uint32_t index) {
				return data_points_[point_map_[index]];
			}

			inline const Vec4f32& point(uint32_t index) const {
				return data_points_[point_map_[index]];
			}

			inline const Vec4ui8& pixel(uint32_t index) const {
				return data_pixels_[point_map_[index]];
			}

			inline uint32_t numValidPoints() const {
				return num_valid_points_;
			}

			inline const Vec4f32& avg() const {
				return avg_;
			}

			inline void boundingBox(Vec4f32& minClr, Vec4f32& maxClr) const {
				minClr = min_clr_;
				maxClr = max_clr_;
			}

			inline bool allSamePoint() const {
				return min_clr_ == max_clr_;
			}

			uint32_t principalAxis(Vec4f32& axis, float32_t* eig_one, float32_t* eig_two) const {
				// We use these vectors for calculating the covariance matrix...
				std::array<Vec4f32, MAX_NUM_DATA_POINTS> to_pts;
				Vec4f32 to_pts_max(-(std::numeric_limits<Vec4f32::ElementType>::max)(), -(std::numeric_limits<Vec4f32::ElementType>::max)(),
					-(std::numeric_limits<Vec4f32::ElementType>::max)(), -(std::numeric_limits<Vec4f32::ElementType>::max)());
				for (auto i = 0u; i < num_valid_points_; ++i) {
					to_pts[i] = point(i) - avg();
					Math::max(to_pts_max.data, to_pts[i].data, to_pts_max.data);
				}

				// Generate a list of unique points...
				std::array<Vec4f32, MAX_NUM_DATA_POINTS> upts;
				auto upts_idx = 0u;
				for (auto i = 0u; i < num_valid_points_; ++i) {
					auto has_pt = false;
					for (auto j = 0u; j < upts_idx; ++j) {
						if (upts[j] == point(i)) {
							has_pt = true;
							break;
						}
					}

					if (!has_pt) {
						upts[upts_idx] = point(i);
						++upts_idx;
					}
				}

				if (1 == upts_idx) {
					axis = 0.f;
					return 0;
				} else {
					auto dir = upts[1] - upts[0];
					dir.normalize();
					bool collinear = true;
					for (auto i = 2u; i < num_valid_points_; ++i) {
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

				Matrix4x4f32 cov_matrix(nullptr);

				// Compute covariance.
				for (auto i = 0u; i < 4u; ++i) {
					for (auto j = 0u; j <= i; ++j) {
						auto sum = 0.f;
						for (auto k = 0u; k < num_valid_points_; ++k) {
							sum += to_pts[k][i] * to_pts[k][j];
						}

						cov_matrix(i, j) = sum / 3.f;
						cov_matrix(j, i) = cov_matrix(i, j);
					}
				}

				auto iters = powerMethod(cov_matrix, axis, eig_one);
				if ((eig_two != nullptr) && (eig_one != nullptr)) {
					if (*eig_one != 0) {
						Matrix4x4f32 reduced(nullptr);
						for (auto j = 0u; j < 4u; ++j) {
							for (auto i = 0u; i < 4u; ++i) {
								reduced(i, j) = axis[j] * axis[i];
							}
						}

						//for (auto i = 0; i < 16; ++i) reduced[i] = cov_matrix[i] - reduced[i] * (*eig_one);
						//reduced = cov_matrix - ((*eig_one) * reduced);
						auto all_zero = true;
						for (auto i = 0u; i < 16u; ++i) {
							if (std::abs(reduced[i]) > 0.0005f) {
								all_zero = false;
								break;
							}
						}

						if (all_zero) {
							*eig_two = 0;
						} else {
							Vec4f32 dummy_dir(nullptr);
							iters += powerMethod(reduced, dummy_dir, eig_two);
						}
					} else {
						*eig_two = 0;
					}
				}

				return iters;
			}

			void shapeIndex(uint32_t shape_index, uint32_t num_partitions) {
				shape_index_ = shape_index;
				num_partitions_ = num_partitions;
			}

			void shapeIndex(uint32_t shape_index) {
				shapeIndex(shape_index, num_partitions_);
			}

			void partition(uint32_t part) {
				selected_partition_ = part;
				recalculate(true);
			}

			bool isPointValid(uint32_t index) const {
				return selected_partition_ == get_partition_(num_partitions_, shape_index_, index);
			}

		private:
			static constexpr auto MAX_NUM_DATA_POINTS = 16u;

			uint32_t num_valid_points_;
			uint32_t num_partitions_;
			uint32_t selected_partition_;
			uint32_t shape_index_;

			Vec4f32 avg_;

			std::array<Vec4f32, MAX_NUM_DATA_POINTS> data_points_;
			std::array<Vec4ui8, MAX_NUM_DATA_POINTS> data_pixels_;
			std::array<uint8_t, MAX_NUM_DATA_POINTS> point_map_;
			Vec4f32 min_clr_;
			Vec4f32 max_clr_;

			std::function<uint32_t(uint32_t, uint32_t, uint32_t)> get_partition_;

			void recalculate(bool consider_valid) {
				num_valid_points_ = 0u;
				avg_ = Vec4f32(VECTOR_SET_ALL, 0);
				min_clr_ = Vec4f32(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
					std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
				max_clr_ = Vec4f32(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
					-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

				auto map = 0u;
				for (size_t i = 0; i < data_points_.size(); ++i) {
					if (consider_valid && !isPointValid(i)) continue;

					const auto& p = data_points_[i];

					++num_valid_points_;
					avg_ += p;
					point_map_[map] = (uint8_t)i;
					++map;

					Math::min(min_clr_.data, p.data, min_clr_.data);
					Math::max(max_clr_.data, p.data, max_clr_.data);
				}

				avg_ /= (float32_t)num_valid_points_;
			}

			uint32_t powerMethod(const Matrix4x4f32& mat, Vec4f32& eig_vec, float* eig_val = nullptr) const {
				static auto const ITER_POWER = 4u;

				Vec4f32 b(VECTOR_SET_ALL, .5f);

				auto bad_eigen_value = false;
				auto fixed = false;
				auto num_iterations = 1u;
				while (!fixed && (num_iterations < ITER_POWER)) {
					Vec4f32 new_b(nullptr);
					mat.transformPoint(b.data, new_b.data);

					// !HACK! If the principal eigenvector of the matrix
					// converges to zero, that could mean that there is no
					// principal eigenvector. However, that may be due to
					// poor initialization of the random vector, so rerandomize
					// and try again.
					auto new_b_len = new_b.getLength();
					if (new_b_len < 1e-6f) {
						if (bad_eigen_value) {
							eig_vec = b;
							if (eig_val) {
								*eig_val = 0;
							}
							return num_iterations;
						}
						
						b.set<false>(1, 1);

						b.normalize();
						bad_eigen_value = true;
						continue;
					}

					new_b.normalize();

					// If the new eigenvector is close enough to the old one,
					// then we've converged.
					if (Math::equal(1.0f, Math::dot(b.data, new_b.data), std::numeric_limits<float32_t>::epsilon())) {
						fixed = true;
					}

					// Save and continue.
					b = new_b;

					++num_iterations;
				}

				// Store the eigenvector in the proper variable.
				eig_vec = b;

				// Store eigenvalue if it was requested
				if (eig_val) {
					Vec4f32 result(nullptr);
					mat.transformPoint(b.data, result.data);
					*eig_val = result.getLength() / b.getLength();
				}

				return num_iterations;
			}
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
				auto threads = new std::thread[threadCount];
				for (decltype(threadCount) i = 0; i < threadCount; ++i) {
					threads[i] = std::thread([&cfg, dataBuffer, i]() {
						_encode(cfg, dataBuffer, i);
						});
				}

				for (decltype(threadCount) i = 0; i < threadCount; ++i) threads[i].join();
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
					Cluster block_cluster((Vec4ui8*)in, 16, [](uint32_t partitions, uint32_t shape, uint32_t offset) {
						return (partitions > 1) ? (BC67_PARTITION_TABLE[partitions - 2][shape] >> (offset * 2)) & 0x3 : 0;
						});
				}

				++startBlock;
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

		static void SRK_CALL _encodeBlock(void* in, void* out) {

		}

		static ShapeSelection SRK_CALL _boxSelection(uint8_t* in) {
			ShapeSelection rst;

			auto opaque = true;
			for (auto i = 0; i <= 16; ++i) {
				auto a = in[(i << 2) + 3];
				opaque = opaque && (a >= 250);
			}

			auto bestErr = (std::numeric_limits<uint64_t>::max)();

			rst.shapes.resize(1);
			rst.shapes[0].numPartitions = 2;
			//for (auto i = 0; i < 64; ++i) {
			//	cluster.ShapeIndex(i, 2);

			//	auto err = 0_ui64;
			//	for (auto ci = 0; ci < 2; ++ci) {
			//		cluster.Partition(ci);
			//		err += EstimateTwoClusterError(metric, cluster);
			//	}

			//	if (err < bestErr) {
			//		bestErr = err;
			//		rst.shapes[0].index = i;
			//	}

			//	// If it's small, we'll take it!
			//	if (err < 1) {
			//		rst.selected_modes = TWO_PARTITION_MODES;
			//		return rst;
			//	}
			//}

			return rst;
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
	};

	//uint8_t BC7Converter::O_MATCH7[256][2] = { {0, 0}, {0, 1}, {0, 3}, {0, 4}, {0, 6}, {0, 7}, {0, 9}, {0, 10}, {0, 12}, {0, 13}, {0, 15}, {0, 16}, {0, 18}, {0, 20}, {0, 21}, {0, 23}, {0, 24}, {0, 26}, {0, 27}, {0, 29}, {0, 30}, {0, 32}, {0, 33}, {0, 35}, {0, 36}, {0, 38}, {0, 39}, {0, 41}, {0, 42}, {0, 44}, {0, 45}, {0, 47}, {0, 48}, {0, 50}, {0, 52}, {0, 53}, {0, 55}, {0, 56}, {0, 58}, {0, 59}, {0, 61}, {0, 62}, {0, 64}, {0, 65}, {0, 66}, {0, 68}, {0, 69}, {0, 71}, {0, 72}, {0, 74}, {0, 75}, {0, 77}, {0, 78}, {0, 80}, {0, 82}, {0, 83}, {0, 85}, {0, 86}, {0, 88}, {0, 89}, {0, 91}, {0, 92}, {0, 94}, {0, 95}, {0, 97}, {0, 98}, {0, 100}, {0, 101}, {0, 103}, {0, 104}, {0, 106}, {0, 107}, {0, 109}, {0, 110}, {0, 112}, {0, 114}, {0, 115}, {0, 117}, {0, 118}, {0, 120}, {0, 121}, {0, 123}, {0, 124}, {0, 126}, {0, 127}, {1, 127}, {2, 126}, {3, 126}, {3, 127}, {4, 127}, {5, 126}, {6, 126}, {6, 127}, {7, 127}, {8, 126}, {9, 126}, {9, 127}, {10, 127}, {11, 126}, {12, 126}, {12, 127}, {13, 127}, {14, 126}, {15, 125}, {15, 127}, {16, 126}, {17, 126}, {17, 127}, {18, 127}, {19, 126}, {20, 126}, {20, 127}, {21, 127}, {22, 126}, {23, 126}, {23, 127}, {24, 127}, {25, 126}, {26, 126}, {26, 127}, {27, 127}, {28, 126}, {29, 126}, {29, 127}, {30, 127}, {31, 126}, {32, 126}, {32, 127}, {33, 127}, {34, 126}, {35, 126}, {35, 127}, {36, 127}, {37, 126}, {38, 126}, {38, 127}, {39, 127}, {40, 126}, {41, 126}, {41, 127}, {42, 127}, {43, 126}, {44, 126}, {44, 127}, {45, 127}, {46, 126}, {47, 125}, {47, 127}, {48, 126}, {49, 126}, {49, 127}, {50, 127}, {51, 126}, {52, 126}, {52, 127}, {53, 127}, {54, 126}, {55, 126}, {55, 127}, {56, 127}, {57, 126}, {58, 126}, {58, 127}, {59, 127}, {60, 126}, {61, 126}, {61, 127}, {62, 127}, {63, 126}, {64, 125}, {64, 126}, {65, 126}, {65, 127}, {66, 127}, {67, 126}, {68, 126}, {68, 127}, {69, 127}, {70, 126}, {71, 126}, {71, 127}, {72, 127}, {73, 126}, {74, 126}, {74, 127}, {75, 127}, {76, 126}, {77, 125}, {77, 127}, {78, 126}, {79, 126}, {79, 127}, {80, 127}, {81, 126}, {82, 126}, {82, 127}, {83, 127}, {84, 126}, {85, 126}, {85, 127}, {86, 127}, {87, 126}, {88, 126}, {88, 127}, {89, 127}, {90, 126}, {91, 126}, {91, 127}, {92, 127}, {93, 126}, {94, 126}, {94, 127}, {95, 127}, {96, 126}, {97, 126}, {97, 127}, {98, 127}, {99, 126}, {100, 126}, {100, 127}, {101, 127}, {102, 126}, {103, 126}, {103, 127}, {104, 127}, {105, 126}, {106, 126}, {106, 127}, {107, 127}, {108, 126}, {109, 125}, {109, 127}, {110, 126}, {111, 126}, {111, 127}, {112, 127}, {113, 126}, {114, 126}, {114, 127}, {115, 127}, {116, 126}, {117, 126}, {117, 127}, {118, 127}, {119, 126}, {120, 126}, {120, 127}, {121, 127}, {122, 126}, {123, 126}, {123, 127}, {124, 127}, {125, 126}, {126, 126}, {126, 127}, {127, 127} };
}

namespace srk::meta {
	template<int64_t I, int64_t N, auto... NonTypeArgs, typename Fn, typename... Args>
	inline void loop(Fn&& fn, Args&&... args) {
		if constexpr (I < N) {
			fn.operator()<I, N, NonTypeArgs...>(std::forward<Args>(args)...);
			loop<I + 1, N, NonTypeArgs...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
		}
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
		auto src = extensions::PNGConverter::decode(readFile(srcDir + "/Resources/red512.png"));
		if (!src) return 0;

		if (src->format == modules::graphics::TextureFormat::R8G8B8) {
			ByteArray dst(src->size.getMultiplies() * 4);
			dst.setLength(dst.getCapacity());
			Image::convertFormat(src->size, src->format, src->source.getSource(), modules::graphics::TextureFormat::R8G8B8A8, dst.getSource());
			src->format = modules::graphics::TextureFormat::R8G8B8A8;
			src->source = std::move(dst);
		}

		meta::loop<0, 2, 3, 4>([]<size_t I, size_t N, size_t A, size_t B>(int a, int b, int c) {
			printaln(I, "  ", N, "  ", A, "  ", B, "  ", c);
			}, 1, 2, 3);

		//src->flipY();
		//auto astc = extensions::ASTCConverter::encode(*src, Vec3ui32(6, 6, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Quality::MEDIUM, extensions::ASTCConverter::Flags::WRITE_HEADER, 12);
		//writeFile(srcDir + "/img.astc", astc);

		//auto bc7 = extensions::BC7Converter::encode(*src, extensions::BC7Converter::Profile::UNORM, extensions::BC7Converter::Quality::Balanced, extensions::BC7Converter::Flags::WRITE_DDS_HEADER, 12);
		//writeFile(srcDir + "/img.dds", bc7);

		return 0;
	}
};