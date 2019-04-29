#pragma once

#include "base/ByteArray.h"
#include "base/Ref.h"
#include "math/Size2.h"
#include "math/Size3.h"
#include <vector>

namespace aurora {
	namespace modules::graphics {
		enum class TextureFormat : ui8;
	}

	class AE_DLL Image : public Ref {
	public:
		Image();

		modules::graphics::TextureFormat format;
		Size2<ui32> size;
		ByteArray source;

		static bool convertFormat(const Size2<ui32>& size, modules::graphics::TextureFormat srcFormat, const ui8* src, modules::graphics::TextureFormat dstFormat, ui8* dst);
		static bool convertFormat(const Size2<ui32>& size, modules::graphics::TextureFormat srcFormat, const ui8* src, ui32 mipLevels, modules::graphics::TextureFormat dstFormat, ui8* dst);

		inline bool generateMips(modules::graphics::TextureFormat format, ui32 mipLevels, ByteArray& dst, std::vector<void*>& dstDataPtr) const {
			dst = ByteArray(calcMipsByteSize(size, mipLevels, calcPerPixelByteSize(format)));
			dstDataPtr.resize(mipLevels);
			return generateMips(format, mipLevels, (ui8*)dst.getBytes(), dstDataPtr.data());
		}
		bool generateMips(modules::graphics::TextureFormat format, ui32 mipLevels, ui8* dst, void** dstDataPtr) const;

		static ui32 calcPerPixelByteSize(modules::graphics::TextureFormat format);

		inline static ui32 AE_CALL calcMipLevels(const Size2<ui32>& size) {
			return calcMipLevels(size.getMax());
		}
		inline static ui32 AE_CALL calcMipLevels(ui32 n) {
			return (ui32)std::floor(std::log2(n) + 1);
		}

		static void AE_CALL calcSpecificMipPixelSize(ui32& size, ui32 mipLevel);
		static void AE_CALL calcSpecificMipPixelSize(Size2<ui32>& size, ui32 mipLevel);

		static std::vector<ui32> AE_CALL calcMipsPixelSize(ui32 n, ui32 mipLevels);

		inline static ui32 calcNextMipPixelSize(ui32 n) {
			return n > 1 ? n >> 1 : 1;
		}
		inline static void calcNextMipPixelSize(Size2<ui32>& size) {
			size.width = calcNextMipPixelSize(size.width);
			size.height = calcNextMipPixelSize(size.height);
		}
		inline static void calcNextMipPixelSize(Size3<ui32>& size) {
			size.width = calcNextMipPixelSize(size.width);
			size.height = calcNextMipPixelSize(size.height);
			size.depth = calcNextMipPixelSize(size.depth);
		}

		inline static ui32 calcByteSize(const Size2<ui32>& size, modules::graphics::TextureFormat format) {
			return calcByteSize(size.getArea(), format);
		}
		inline constexpr static ui32 calcByteSize(const Size2<ui32>& size, ui32 perPixelByteSize) {
			return calcByteSize(size.getArea(), perPixelByteSize);
		}
		inline static ui32 calcByteSize(ui32 numPixels, modules::graphics::TextureFormat format) {
			return calcByteSize(numPixels, calcPerPixelByteSize(format));
		}
		inline constexpr static ui32 calcByteSize(ui32 numPixels, ui32 perPixelByteSize) {
			return numPixels * perPixelByteSize;
		}

		static ui32 calcMipsByteSize(const Size2<ui32>& size, ui32 mipLevels, ui32 perPixelByteSize);
		static ui32 calcMipsByteSize(const Size3<ui32>& size, ui32 mipLevels, ui32 perPixelByteSize);

		static void generateMips_UInt8s(const Size2<ui32>& size, modules::graphics::TextureFormat format, ui32 mipLevels, ui8 numChannels, ui8* dst, void** dataPtr);

	private:
		static void _convertFormat_R8G8B8_R8G8B8A8(const Size2<ui32>& size, const ui8* src, ui8* dst);
		static void _convertFormat_R8G8B8A8_R8G8B8(const Size2<ui32>& size, const ui8* src, ui8* dst);

		static void _mipPixelsBlend(ui8* c, ui8 numChannels, ui8 numPixels);
	};
}