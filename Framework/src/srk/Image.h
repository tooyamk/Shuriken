#pragma once

#include "srk/ByteArray.h"
#include "srk/Intrusive.h"
#include "srk/math/Vector.h"
#include "srk/modules/graphics/GraphicsModule.h"
#include <vector>

namespace srk {
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

		template<SameAnyOf<size_t, Vec2uz, Vec3uz> T>
		static size_t SRK_CALL getSpecificMipPixels(const T& curPixels, size_t curMipLevel, size_t dstMipLevel) {
			if (curMipLevel < dstMipLevel) {
				auto pixels = curPixels;
				for (size_t i = curMipLevel; i < dstMipLevel; ++i) pixels = modules::graphics::TextureUtils::getNextMipPixels(pixels);
				return pixels;
			} else {
				return curPixels << (curMipLevel - dstMipLevel);
			}
		}

		static std::vector<size_t> SRK_CALL calcMipsPixels(size_t n, size_t mipLevels);

		static void SRK_CALL generateMips_UInt8s(const Vec2uz& pixels, modules::graphics::TextureFormat format, size_t mipLevels, size_t numChannels, void* dst, void** dstMipData);

		bool SRK_CALL flipY();

		bool SRK_CALL scale(Image& dst) const;
		bool SRK_CALL scale(Image& dst, size_t jobCount, size_t threadCount, size_t threadIndex) const;

	private:
		static void SRK_CALL _mipPixelsBlend(uint8_t* c, uint8_t numChannels, uint8_t numPixels);
	};
}