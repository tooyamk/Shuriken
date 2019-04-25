#pragma once

#include "base/ByteArray.h"
#include "modules/IGraphicsModule.h"

namespace aurora {
	class AE_DLL Image : public Ref {
	public:
		Image();

		modules::graphics::TextureFormat format;
		ui32 width;
		ui32 height;
		ByteArray source;

		static bool convertFormat(ui32 width, ui32 height, modules::graphics::TextureFormat srcFormat, const ui8* src, modules::graphics::TextureFormat dstFormat, ui8* dst);
		static bool convertFormat(ui32 width, ui32 height, modules::graphics::TextureFormat srcFormat, const ui8* src, ui32 mipLevels, modules::graphics::TextureFormat dstFormat, ui8* dst);

		inline bool generateMips(modules::graphics::TextureFormat format, ui32 mipLevels, ByteArray& dst, std::vector<void*>& dstDataPtr) const {
			dst = ByteArray(calcMipsByteSize(width, height, mipLevels, format));
			dstDataPtr.resize(mipLevels);
			return generateMips(format, mipLevels, (ui8*)dst.getBytes(), dstDataPtr.data());
		}
		bool generateMips(modules::graphics::TextureFormat format, ui32 mipLevels, ui8* dst, void** dstDataPtr) const;

		static ui32 calcPerPixelByteSize(modules::graphics::TextureFormat format);

		inline static ui32 AE_CALL calcMipLevels(const Image& img) {
			return calcMipLevels(std::max<ui32>(img.width, img.height));
		}
		inline static ui32 AE_CALL calcMipLevels(ui32 width, ui32 height) {
			return calcMipLevels(std::max<ui32>(width, height));
		}
		inline static ui32 AE_CALL calcMipLevels(ui32 n) {
			return (ui32)std::floor(std::log2(n) + 1);
		}

		static std::vector<ui32> AE_CALL calcMipsPixelSize(ui32 n, ui32 mipLevels);
		inline static ui32 calcNextMipPixelSize(ui32 n) {
			if (n > 1) {
				if (n & 0b1) ++n;
				n >>= 1;
			}
			return n;
		}

		inline static ui32 calcByteSize(ui32 width, ui32 height, modules::graphics::TextureFormat format) {
			return calcByteSize(width * height, format);
		}
		inline constexpr static ui32 calcByteSize(ui32 width, ui32 height, ui32 perPixelByteSize) {
			return calcByteSize(width * height, perPixelByteSize);
		}
		inline static ui32 calcByteSize(ui32 numPixels, modules::graphics::TextureFormat format) {
			return calcByteSize(numPixels, calcPerPixelByteSize(format));
		}
		inline constexpr static ui32 calcByteSize(ui32 numPixels, ui32 perPixelByteSize) {
			return numPixels * perPixelByteSize;
		}

		inline static ui32 calcMipsByteSize(ui32 width, ui32 height, ui32 mipLevels, modules::graphics::TextureFormat format) {
			return calcMipsByteSize(width, height, mipLevels, calcPerPixelByteSize(format));
		}
		static ui32 calcMipsByteSize(ui32 width, ui32 height, ui32 mipLevels, ui32 perPixelByteSize);

		static void generateMips_UInt8s(ui32 width, ui32 height, modules::graphics::TextureFormat format, ui32 mipLevels, ui8 numChannels, ui8* dst, void** dataPtr);

	private:
		static void _convertFormat_R8G8B8_R8G8B8A8(ui32 width, ui32 height, const ui8* src, ui8* dst);

		static void _mipPixelsBlend(ui8* c, ui8 numChannels, ui8 numPixels);
	};
}