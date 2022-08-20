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

		/*class Cluster {
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

			uint32_t principalAxis(Vec4f32& axis, float* eig_one, float* eig_two) const {
				// We use these vectors for calculating the covariance matrix...
				std::array<Vec4f32, MAX_NUM_DATA_POINTS> to_pts;
				Vec4f32 to_pts_max(-(std::numeric_limits<Vec4f32::ElementType>::max)(), -(std::numeric_limits<Vec4f32::ElementType>::max)(),
					-(std::numeric_limits<Vec4f32::ElementType>::max)(), -(std::numeric_limits<Vec4f32::ElementType>::max)());
				for (uint32_t i = 0; i < num_valid_points_; ++i) {
					to_pts[i] = point(i) - avg();
					Math::max(to_pts_max.data, to_pts[i].data, to_pts_max.data);
				}

				// Generate a list of unique points...
				std::array<Vec4f32, MAX_NUM_DATA_POINTS> upts;
				uint32_t upts_idx = 0;
				for (uint32_t i = 0; i < num_valid_points_; ++i) {
					auto has_pt = false;
					for (uint32_t j = 0; j < upts_idx; ++j) {
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
					axis.setAll(0.0f);
					return 0;
				} else {
					auto dir = upts[1] - upts[0];
					dir.normalize();
					bool collinear = true;
					for (uint32_t i = 2; i < num_valid_points_; ++i) {
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

				Matrix44 cov_matrix;

				// Compute covariance.
				for (uint32_t i = 0; i < 4; ++i) {
					for (uint32_t j = 0; j <= i; ++j) {
						float32_t sum = 0;
						for (uint32_t k = 0; k < num_valid_points_; ++k) {
							sum += to_pts[k][i] * to_pts[k][j];
						}

						cov_matrix(i, j) = sum / 3;
						cov_matrix(j, i) = cov_matrix(i, j);
					}
				}

				Math::aaa<2>(1);

				uint32_t iters = powerMethod(cov_matrix, axis, eig_one);
				if ((eig_two != nullptr) && (eig_one != nullptr)) {
					if (*eig_one != 0) {
						Matrix44 reduced;
						for (uint32_t j = 0; j < 4; ++j) {
							for (uint32_t i = 0; i < 4; ++i) {
								reduced(i, j) = axis[j] * axis[i];
							}
						}

						reduced = cov_matrix - ((*eig_one) * reduced);
						bool all_zero = true;
						for (uint32_t i = 0; i < 16; ++i) {
							if (std::abs(reduced[i]) > 0.0005f) {
								all_zero = false;
								break;
							}
						}

						if (all_zero) {
							*eig_two = 0;
						} else {
							Vec4f32 dummy_dir;
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
			static constexpr int MAX_NUM_DATA_POINTS = 16;

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
				num_valid_points_ = 0;
				avg_ = float4(0, 0, 0, 0);
				min_clr_ = float4(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
					std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
				max_clr_ = float4(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
					-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

				uint32_t map = 0;
				for (uint32_t i = 0; i < data_points_.size(); ++i) {
					if (consider_valid && !this->IsPointValid(i)) {
						continue;
					}

					float4 const& p = data_points_[i];

					++num_valid_points_;
					avg_ += p;
					point_map_[map] = static_cast<uint8_t>(i);
					++map;

					min_clr_ = MathLib::minimize(min_clr_, p);
					max_clr_ = MathLib::maximize(max_clr_, p);
				}

				avg_ /= static_cast<float>(num_valid_points_);
			}
			int powerMethod(const float4x4& mat, Vec4f32& eig_vec, float* eig_val = nullptr) const {
				static int const ITER_POWER = 4;

				float4 b;
				float norm = 0.5f;
				for (int i = 0; i < 4; ++i) {
					b[i] = norm;
				}

				bool bad_eigen_value = false;
				bool fixed = false;
				int num_iterations = 1;
				while (!fixed && (num_iterations < ITER_POWER)) {
					float4 new_b = MathLib::transform(b, mat);

					// !HACK! If the principal eigenvector of the matrix
					// converges to zero, that could mean that there is no
					// principal eigenvector. However, that may be due to
					// poor initialization of the random vector, so rerandomize
					// and try again.
					float const new_b_len = MathLib::length(new_b);
					if (new_b_len < 1e-6f) {
						if (bad_eigen_value) {
							eig_vec = b;
							if (eig_val) {
								*eig_val = 0;
							}
							return num_iterations;
						}

						for (int i = 0; i < 2; ++i) {
							b[i] = 1;
						}

						b = MathLib::normalize(b);
						bad_eigen_value = true;
						continue;
					}

					new_b = MathLib::normalize(new_b);

					// If the new eigenvector is close enough to the old one,
					// then we've converged.
					if (MathLib::equal(1.0f, MathLib::dot(b, new_b))) {
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
					float4 result = MathLib::transform(b, mat);
					*eig_val = MathLib::length(result) / MathLib::length(b);
				}

				return num_iterations;
			}
		};*/

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
		static uint8_t O_MATCH7[256][2];

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
			/*rst.shapes[0].numPartitions = 2;
			for (auto i = 0; i < 64; ++i) {
				cluster.ShapeIndex(i, 2);

				auto err = 0_ui64;
				for (auto ci = 0; ci < 2; ++ci) {
					cluster.Partition(ci);
					err += EstimateTwoClusterError(metric, cluster);
				}

				if (err < bestErr) {
					bestErr = err;
					rst.shapes[0].index = i;
				}

				// If it's small, we'll take it!
				if (err < 1) {
					rst.selected_modes = TWO_PARTITION_MODES;
					return rst;
				}
			}*/

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

	uint8_t BC7Converter::O_MATCH7[256][2] = { {0, 0}, {0, 1}, {0, 3}, {0, 4}, {0, 6}, {0, 7}, {0, 9}, {0, 10}, {0, 12}, {0, 13}, {0, 15}, {0, 16}, {0, 18}, {0, 20}, {0, 21}, {0, 23}, {0, 24}, {0, 26}, {0, 27}, {0, 29}, {0, 30}, {0, 32}, {0, 33}, {0, 35}, {0, 36}, {0, 38}, {0, 39}, {0, 41}, {0, 42}, {0, 44}, {0, 45}, {0, 47}, {0, 48}, {0, 50}, {0, 52}, {0, 53}, {0, 55}, {0, 56}, {0, 58}, {0, 59}, {0, 61}, {0, 62}, {0, 64}, {0, 65}, {0, 66}, {0, 68}, {0, 69}, {0, 71}, {0, 72}, {0, 74}, {0, 75}, {0, 77}, {0, 78}, {0, 80}, {0, 82}, {0, 83}, {0, 85}, {0, 86}, {0, 88}, {0, 89}, {0, 91}, {0, 92}, {0, 94}, {0, 95}, {0, 97}, {0, 98}, {0, 100}, {0, 101}, {0, 103}, {0, 104}, {0, 106}, {0, 107}, {0, 109}, {0, 110}, {0, 112}, {0, 114}, {0, 115}, {0, 117}, {0, 118}, {0, 120}, {0, 121}, {0, 123}, {0, 124}, {0, 126}, {0, 127}, {1, 127}, {2, 126}, {3, 126}, {3, 127}, {4, 127}, {5, 126}, {6, 126}, {6, 127}, {7, 127}, {8, 126}, {9, 126}, {9, 127}, {10, 127}, {11, 126}, {12, 126}, {12, 127}, {13, 127}, {14, 126}, {15, 125}, {15, 127}, {16, 126}, {17, 126}, {17, 127}, {18, 127}, {19, 126}, {20, 126}, {20, 127}, {21, 127}, {22, 126}, {23, 126}, {23, 127}, {24, 127}, {25, 126}, {26, 126}, {26, 127}, {27, 127}, {28, 126}, {29, 126}, {29, 127}, {30, 127}, {31, 126}, {32, 126}, {32, 127}, {33, 127}, {34, 126}, {35, 126}, {35, 127}, {36, 127}, {37, 126}, {38, 126}, {38, 127}, {39, 127}, {40, 126}, {41, 126}, {41, 127}, {42, 127}, {43, 126}, {44, 126}, {44, 127}, {45, 127}, {46, 126}, {47, 125}, {47, 127}, {48, 126}, {49, 126}, {49, 127}, {50, 127}, {51, 126}, {52, 126}, {52, 127}, {53, 127}, {54, 126}, {55, 126}, {55, 127}, {56, 127}, {57, 126}, {58, 126}, {58, 127}, {59, 127}, {60, 126}, {61, 126}, {61, 127}, {62, 127}, {63, 126}, {64, 125}, {64, 126}, {65, 126}, {65, 127}, {66, 127}, {67, 126}, {68, 126}, {68, 127}, {69, 127}, {70, 126}, {71, 126}, {71, 127}, {72, 127}, {73, 126}, {74, 126}, {74, 127}, {75, 127}, {76, 126}, {77, 125}, {77, 127}, {78, 126}, {79, 126}, {79, 127}, {80, 127}, {81, 126}, {82, 126}, {82, 127}, {83, 127}, {84, 126}, {85, 126}, {85, 127}, {86, 127}, {87, 126}, {88, 126}, {88, 127}, {89, 127}, {90, 126}, {91, 126}, {91, 127}, {92, 127}, {93, 126}, {94, 126}, {94, 127}, {95, 127}, {96, 126}, {97, 126}, {97, 127}, {98, 127}, {99, 126}, {100, 126}, {100, 127}, {101, 127}, {102, 126}, {103, 126}, {103, 127}, {104, 127}, {105, 126}, {106, 126}, {106, 127}, {107, 127}, {108, 126}, {109, 125}, {109, 127}, {110, 126}, {111, 126}, {111, 127}, {112, 127}, {113, 126}, {114, 126}, {114, 127}, {115, 127}, {116, 126}, {117, 126}, {117, 127}, {118, 127}, {119, 126}, {120, 126}, {120, 127}, {121, 127}, {122, 126}, {123, 126}, {123, 127}, {124, 127}, {125, 126}, {126, 126}, {126, 127}, {127, 127} };
}

namespace srk {
	enum class MatrixHint : uint8_t {
		NONE = 0,
		IDENTITY_OTHERS = 1 << 0
	};

	/**
	 *
	 */
	template<size_t Rows, size_t Columns, std::floating_point T>
	class Matrix {
	public:
		static constexpr size_t ROWS = Rows;
		static constexpr size_t COLUMNS = Columns;
		using ElementType = T;
		using Data = T[ROWS][COLUMNS];

		Matrix() {
			identity();
		}

		Matrix(nullptr_t) {}

		Matrix(const Matrix& m) {
			memccpy(data, m.data, sizeof(Data));
		}

		Matrix(Matrix&& m) {
			memccpy(data, m.data, sizeof(Data));
		}

		template<size_t R, size_t C, std::floating_point K>
		Matrix(const Matrix<R, C, K>& m) : Matrix(m.data) {}

		template<size_t R, size_t C, std::floating_point K>
		Matrix(Matrix<R, C, K>&& m) : Matrix(m.data) {}

		template<size_t R, size_t C, std::floating_point K>
		Matrix(const K(&val)[R][C]) {
			set(val);
		}

		template<std::convertible_to<T>... Args>
		requires (sizeof...(Args) > 0)
		Matrix(Args&&... args) {
			set(std::forward<Args>(args)...);
		}

		inline SRK_CALL operator Data& () {
			return data;
		}
		inline SRK_CALL operator const Data& () const {
			return data;
		}

		template<std::integral Row, std::integral Column>
		inline T& SRK_CALL operator()(Row r, Column c) {
			return data[r][c];
		}
		template<std::integral Row, std::integral Column>
		inline const T& SRK_CALL operator()(Row r, Column c) const {
			return data[r][c];
		}

		inline constexpr size_t SRK_CALL getRows() const {
			return ROWS;
		}

		inline constexpr size_t SRK_CALL getColumns() const {
			return COLUMNS;
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = nullptr>
		inline void SRK_CALL identity() {
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::identity<ddesc>(data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t SrcR, size_t SrcC, std::floating_point SrcT>
		inline void SRK_CALL set(const SrcT(&src)[SrcR][SrcC]) {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);
			
			Math::copy<sdesc, ddesc>(src, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t SrcR, size_t SrcC, std::floating_point SrcT>
		inline void SRK_CALL set(const Matrix<SrcR, SrcC, SrcT>& src) {
			set<Hints, SrcDesc, DstDesc>(src.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, size_t SrcTotalRs = ROWS, size_t SrcTotalCs = COLUMNS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::convertible_to<T>... Args>
		inline void SRK_CALL set(Args&&... args) {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::copy<sdesc, SrcTotalCs, SrcTotalRs, ddesc>(data, std::forward<Args>(args)...);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = nullptr>
		inline void SRK_CALL transpose() {
			using namespace srk::enum_operators;

			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::transpose<sdesc, ddesc>(data, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t DstR, size_t DstC, std::floating_point DstT>
		inline void SRK_CALL transpose(const DstT(&dst)[DstR][DstC]) const {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::transpose<sdesc, ddesc>(data, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t DstR, size_t DstC, std::floating_point DstT>
		inline void SRK_CALL transpose(Matrix<DstR, DstC, DstT>& dst) const {
			transpose<Hints, SrcDesc, DstDesc>(dst.data);
		}

		inline bool SRK_CALL invert() {
			return Math::invert<Math::Hint::MEM_OVERLAP>(data, data);
		}

		template<MatrixHint Hints = MatrixHint::NONE, size_t DstR, size_t DstC, std::floating_point DstT>
		inline bool SRK_CALL invert(DstT(&dst)[DstR][DstC]) const {
			return Math::invert<(Math::Hint)Hints>(data, dst);
		}

		template<MatrixHint Hints = MatrixHint::NONE, size_t DstR, size_t DstC, std::floating_point DstT>
		inline bool SRK_CALL invert(Matrix<DstR, DstC, DstT>& m) const {
			return invert<Hints>(m.data);
		}

		template<Math::Data2DDesc SrcDesc = nullptr, Math::Data2DDesc DstRotDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, typename RT, typename ST>
		requires ((std::same_as<RT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<RT>> && !std::is_const_v<std::remove_reference_t<RT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<RT>>> && std::rank_v<std::remove_cvref_t<RT>> == 2)) &&
			(std::same_as<ST, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<ST>> && !std::is_const_v<std::remove_reference_t<ST>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<ST>>> && std::rank_v<std::remove_cvref_t<ST>> == 1)))
		inline void SRK_CALL decompose(RT&& dstRot, ST&& dstScale) const {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto drdesc = Math::Data2DDesc(Math::DataType::MATRIX, DstRotDesc);

			Math::decompose<sdesc, drdesc>(data, std::forward<RT>(dstRot), std::forward<ST>(dstScale));
		}

		template<Math::Data2DDesc SrcDesc = nullptr, Math::DataDesc DstDesc = nullptr, size_t DN, std::floating_point DT>
		inline void SRK_CALL toQuaternion(DT(&dst)[DN]) const {
			constexpr auto sdesc = Math::Data2DDesc(Math::DataType::MATRIX, SrcDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::QUATERNION, DstDesc);

			Math::transform<sdesc, ddesc>(data, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT>
		inline void SRK_CALL append(const LT(&lhs)[LRs][LCs]) {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT>
		inline void SRK_CALL append(const Matrix<LRs, LCs, LT>& lhs) {
			append<Hints, LDesc, RDesc, DDesc>(lhs.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const LT(&lhs)[LRs][LCs], DT(&dst)[DRs][DCs]) const {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const Matrix<LRs, LCs, LT>& lhs, DT(&dst)[DRs][DCs]) const {
			append<Hints, LDesc, RDesc, DDesc>(lhs.data, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const LT(&lhs)[LRs][LCs], Matrix<DRs, DCs, DT>& dst) const {
			append<Hints, LDesc, RDesc, DDesc>(lhs, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t LRs, size_t LCs, std::floating_point LT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL append(const Matrix<LRs, LCs, LT>& lhs, Matrix<DRs, DCs, DT>& dst) const {
			append<Hints, LDesc, RDesc, DDesc>(lhs.data, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT>
		inline void SRK_CALL prepend(const RT(&rhs)[RRs][RCs]) {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT>
		inline void SRK_CALL prepend(const Matrix<RRs, RCs, RT>& rhs) {
			prepend<Hints, LDesc, RDesc, DDesc>(rhs.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const RT(&rhs)[RRs][RCs], DT(&dst)[DRs][DCs]) const {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::MATRIX, LDesc);
			constexpr auto rdesc = Math::Data2DDesc(Math::DataType::MATRIX, RDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DDesc);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, dst);
			_autoIdentity<Hints, ddesc>(dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const Matrix<RRs, RCs, RT>& rhs, DT(&dst)[DRs][DCs]) const {
			prepend<Hints, LDesc, RDesc, DDesc>(rhs.data, dst);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const RT(&rhs)[RRs][RCs], Matrix<DRs, DCs, DT>& dst) const {
			prepend<Hints, LDesc, RDesc, DDesc>(rhs, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc LDesc = nullptr, Math::Data2DDesc RDesc = nullptr, Math::Data2DDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RRs, size_t RCs, std::floating_point RT, size_t DRs, size_t DCs, std::floating_point DT>
		inline void SRK_CALL prepend(const Matrix<RRs, RCs, RT>& rhs, Matrix<DRs, DCs, DT>& dst) const {
			prepend<Hints, LDesc, RDesc, DDesc>(rhs.data, dst.data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point FwdT, std::floating_point UwdT>
		inline void SRK_CALL lookAt(const FwdT(&forward)[3], const UwdT(&upward)[3]) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::lookAt<ddesc>(forward, upward, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point AxisT, std::floating_point RadT>
		inline void SRK_CALL rotationAxis(const AxisT(&axis)[3], RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotationAxis<ddesc>(axis, radian, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline void SRK_CALL rotationX(RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotationX<ddesc>(radian, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline void SRK_CALL rotationY(RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotationY<ddesc>(radian, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline void SRK_CALL rotationZ(RadT radian) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::rotationZ<ddesc>(radian, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc ScaleDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t SN, std::floating_point ST>
		inline void SRK_CALL scale(const ST(&s)[SN]) {
			using namespace srk::enum_operators;

			constexpr auto sdesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, ScaleDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::scale<sdesc, ddesc>(s, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point TT>
		inline void SRK_CALL translation(const TT(&t)[3]) {
			using namespace srk::enum_operators;

			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc.hints | Math::Hint::MEM_OVERLAP, DstDesc.range);

			Math::translation<ddesc>(t, data);
			_autoIdentity<Hints, ddesc>(data);
		}

		template<MatrixHint Hints = MatrixHint::IDENTITY_OTHERS, Math::DataDesc TDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc SDesc = nullptr, Math::Data2DDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, typename TT, typename RT, typename ST, size_t DstR, size_t DstC, std::floating_point DstT>
		requires ((std::same_as<TT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<TT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<TT>>> && std::rank_v<std::remove_cvref_t<TT>> == 1)) &&
			(std::same_as<RT, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<RT>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<RT>>> && std::rank_v<std::remove_cvref_t<RT>> == 1)) &&
			(std::same_as<ST, nullptr_t> || (std::is_bounded_array_v<std::remove_cvref_t<ST>> && std::floating_point<std::remove_all_extents_t<std::remove_cvref_t<ST>>> && std::rank_v<std::remove_cvref_t<ST>> == 1)))
		inline void SRK_CALL trs(TT&& trans, RT&& rot, ST&& scale) {
			using namespace srk::enum_operators;

			constexpr auto tdesc = Math::DataDesc(Math::DataType::VECTOR, TDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::QUATERNION, RDesc);
			constexpr auto sdesc = Math::DataDesc(Math::DataType::MATRIX_SCALE, SDesc);
			constexpr auto ddesc = Math::Data2DDesc(Math::DataType::MATRIX, DstDesc);

			Math::trs<tdesc, rdesc, sdesc, ddesc>(std::forward<TT>(trans), std::forward<RT>(rot), std::forward<ST>(scale), data);
			_autoIdentity<Hints, ddesc>(data);
		}

		union {
			//__m128 col[4];
			Data data;
		};

	private:
		template<MatrixHint Hints, Math::Data2DDesc Desc, size_t Rs, size_t Cs, std::floating_point T>
		inline static void SRK_CALL _autoIdentity(T(&dst)[Rs][Cs]) {
			using namespace srk::enum_operators;

			if constexpr ((Hints & MatrixHint::IDENTITY_OTHERS) == MatrixHint::IDENTITY_OTHERS) {
				if constexpr ((Desc.hints & Math::Hint::OUTSIDE) == Math::Hint::OUTSIDE) {
					Math::identity<Math::Data2DDesc(Desc, Desc.hints & (~Math::Hint::OUTSIDE))>(dst);
				} else {
					Math::identity<Math::Data2DDesc(Desc, Desc.hints | Math::Hint::OUTSIDE)>(dst);
				}
			}
		}
	};

	template<typename T> using Matrix3x3 = Matrix<3, 3, T>;
	using Matrix3x3f32 = Matrix3x3<float32_t>;
	template<typename T> using Matrix3x4 = Matrix<3, 4, T>;
	using Matrix3x4f32 = Matrix3x4<float32_t>;
	template<typename T> using Matrix4x4 = Matrix<4, 4, T>;
	using Matrix4x4f32 = Matrix4x4<float32_t>;

	class MatrixUtils {
	public:
		MatrixUtils() = delete;

		
	};
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

		Matrix34 m1;
		Matrix34 m2;

		Matrix34 trans;
		trans.setPosition(1, 2, 3);
		Math::mul(trans.data, m1.data, m1.data);

		Matrix34 r;
		Matrix34::createRotationY(Math::rad(90.0f), r);
		Math::mul(r.data, m2.data, m2.data);

		Matrix34 w;
		Math::mul(m1.data, m2.data, w.data);

		//m1.appendTranslate(Vec3f32(1, 2, 3));

		//Matrix34 m;
		//Math::mul(m.data, m1.data, m.data);

		Matrix4x4f32 m44;

		Matrix3x4f32 m34(m44);
		m34(0, 3) = 1;
		m34(1, 3) = 2;
		m34(2, 3) = 3;

		Matrix3x4f32 m34p;

		//float32_t m34[3][4];
		//float32_t m34p[3][4];
		float32_t p[3][1] = { 0, 0, 0 };
		//MathUtils::copy<Math::Hint::NONE, nullptr, nullptr>(m34.data, p);
		m44.transpose();
		Math::mul<Math::DataType::MATRIX, Math::DataType::MATRIX, Math::Data2DDesc(Math::DataType::MATRIX, 0, 3, 0, 0, 3, 1)>(m34.data, m34p.data, p);
		//testaa::mul<testaa::Range()>();

		//src->flipY();
		//auto astc = extensions::ASTCConverter::encode(*src, Vec3ui32(6, 6, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Quality::MEDIUM, extensions::ASTCConverter::Flags::WRITE_HEADER, 12);
		//writeFile(srcDir + "/img.astc", astc);

		//auto bc7 = extensions::BC7Converter::encode(*src, extensions::BC7Converter::Profile::UNORM, extensions::BC7Converter::Quality::Balanced, extensions::BC7Converter::Flags::WRITE_DDS_HEADER, 12);
		//writeFile(srcDir + "/img.dds", bc7);

		return 0;
	}
};