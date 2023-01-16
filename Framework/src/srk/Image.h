#pragma once

#include "srk/ByteArray.h"
#include "srk/Intrusive.h"
#include "srk/math/Vector.h"
#include <vector>

namespace srk {
	namespace modules::graphics {
		enum class TextureFormat : uint8_t;
	}

	class SRK_FW_DLL Image {
		SRK_REF_OBJECT(Image)
	public:
		Image();
		Image(Image&& other) noexcept;

		Image& SRK_CALL operator=(Image&& other) noexcept;

		modules::graphics::TextureFormat format;
		Vec2uz dimensions;
		ByteArray source;

		static bool SRK_CALL convertFormat(const Vec2uz& pixels, modules::graphics::TextureFormat srcFormat, const void* src, modules::graphics::TextureFormat dstFormat, void* dst, size_t* srcReadedBytes = nullptr, size_t* dstWritedBytes = nullptr);
		static bool SRK_CALL convertFormat(const Vec2uz& pixels, modules::graphics::TextureFormat srcFormat, const void* src, size_t mipLevels, modules::graphics::TextureFormat dstFormat, void* dst);

		bool SRK_CALL generateMips(modules::graphics::TextureFormat format, size_t mipLevels, ByteArray& dst, size_t dstOffset, std::vector<void*>& dstMipData) const;
		bool SRK_CALL generateMips(modules::graphics::TextureFormat format, size_t mipLevels, void* dst, void** dstMipData) const;

		static bool SRK_CALL isCompressedFormat(modules::graphics::TextureFormat format);

		static size_t SRK_CALL calcBlocks(modules::graphics::TextureFormat format, size_t pixels);
		static Vec2uz SRK_CALL calcBlocks(modules::graphics::TextureFormat format, const Vec2uz& pixels);
		static Vec3uz SRK_CALL calcBlocks(modules::graphics::TextureFormat format, const Vec3uz& pixels);
		static size_t SRK_CALL calcPerBlockBytes(modules::graphics::TextureFormat format);
		static Vec2uz SRK_CALL calcPerBlockPixels(modules::graphics::TextureFormat format);

		//static uint32_t SRK_CALL calcPerPixelByteSize(modules::graphics::TextureFormat format);

		inline static size_t SRK_CALL calcMipLevels(const Vec2uz& pixels) {
			return calcMipLevels(pixels.getMax());
		}
		inline static size_t SRK_CALL calcMipLevels(size_t n) {
			return (size_t)std::floor(std::log2(n) + 1);
		}

		inline static size_t SRK_CALL calcSpecificMipPixels(size_t mip0Pixels, size_t mipLevel) {
			size_t pixels = mip0Pixels;
			for (size_t i = 0; i < mipLevel; ++i) pixels = calcNextMipPixels(pixels);
			return pixels;
		}
		inline static Vec2uz SRK_CALL calcSpecificMipPixels(const Vec2uz& mip0Pixels, size_t mipLevel) {
			Vec2uz pixels = mip0Pixels;
			for (size_t i = 0; i < mipLevel; ++i) pixels = calcNextMipPixels(pixels);
			return pixels;
		}
		inline static Vec3uz SRK_CALL calcSpecificMipPixels(const Vec3uz& mip0Pixels, size_t mipLevel) {
			Vec3uz pixels = mip0Pixels;
			for (size_t i = 0; i < mipLevel; ++i) pixels = calcNextMipPixels(pixels);
			return pixels;
		}

		static std::vector<size_t> SRK_CALL calcMipsPixels(size_t n, size_t mipLevels);

		inline static size_t SRK_CALL calcNextMipPixels(size_t n) {
			return n > 1 ? n >> 1 : 1;
		}
		inline static Vec2uz SRK_CALL calcNextMipPixels(const Vec2uz& pixels) {
			return Vec2uz(calcNextMipPixels(pixels[0]), calcNextMipPixels(pixels[1]));
		}
		inline static Vec3uz SRK_CALL calcNextMipPixels(const Vec3uz& pixels) {
			return Vec3uz(calcNextMipPixels(pixels[0]), calcNextMipPixels(pixels[1]), calcNextMipPixels(pixels[2]));
		}

		inline static size_t SRK_CALL calcBytes(modules::graphics::TextureFormat format, const Vec2uz& pixels) {
			return calcBytes(calcPerBlockBytes(format), calcBlocks(format, pixels).getMultiplies());
		}
		inline static size_t SRK_CALL calcBytes(modules::graphics::TextureFormat format, size_t blocks) {
			return calcBytes(calcPerBlockBytes(format), blocks);
		}
		inline constexpr static size_t SRK_CALL calcBytes(size_t perBlockBytes, size_t blocks) {
			return blocks * perBlockBytes;
		}

		static void SRK_CALL calcMipsInfo(modules::graphics::TextureFormat format, const Vec2uz& pixels, size_t mipLevels, size_t* totalBytes = nullptr, size_t* mipBytes = nullptr, Vec2uz* mipDimensions = nullptr);
		static void SRK_CALL calcMipsInfo(modules::graphics::TextureFormat format, const Vec3uz& pixels, size_t mipLevels, size_t* totalBytes = nullptr, size_t* mipBytes = nullptr, Vec3uz* mipDimensions = nullptr);

		static void SRK_CALL generateMips_UInt8s(const Vec2uz& pixels, modules::graphics::TextureFormat format, size_t mipLevels, size_t numChannels, void* dst, void** dstMipData);

		bool SRK_CALL flipY();

		bool SRK_CALL scale(Image& dst) const;
		bool SRK_CALL scale(Image& dst, size_t jobCount, size_t threadCount, size_t threadIndex) const;

	private:
		static void SRK_CALL _convertFormat_R8G8B8_R8G8B8A8(const Vec2uz& pixels, const void* src, void* dst, size_t* srcReadedBytes, size_t* dstWritedBytes);
		static void SRK_CALL _convertFormat_R8G8B8A8_R8G8B8(const Vec2uz& pixels, const void* src, void* dst, size_t* srcReadedBytes, size_t* dstWritedBytes);

		static void SRK_CALL _mipPixelsBlend(uint8_t* c, uint8_t numChannels, uint8_t numPixels);
	};
}