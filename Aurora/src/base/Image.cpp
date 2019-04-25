#include "Image.h"

namespace aurora {
	//using namespace modules;
	using namespace modules::graphics;

	Image::Image() :
		format(TextureFormat::UNKNOWN),
		width(0),
		height(0) {
	}

	ui32 Image::calcPerPixelByteSize(TextureFormat format) {
		switch (format) {
		case TextureFormat::R8G8B8:
			return 3;
		case TextureFormat::R8G8B8A8:
			return 4;
		default:
			return 0;
		}
	}

	std::vector<ui32> Image::calcMipsPixelSize(ui32 n, ui32 mipLevels) {
		if (!n) return std::move(std::vector<ui32>(0));
		if (!mipLevels) mipLevels = calcMipLevels(n);

		std::vector<ui32> levels(mipLevels);
		levels[0] = n;

		for (ui32 i = 1; i < mipLevels; ++i) {
			n = calcNextMipPixelSize(n);
			levels[i] = n;
		}

		return std::move(levels);
	}

	ui32 Image::calcMipsByteSize(ui32 width, ui32 height, ui32 mipLevels, ui32 perPixelByteSize) {
		auto widths = calcMipsPixelSize(width, mipLevels);
		auto heights = calcMipsPixelSize(height, mipLevels);
		ui32 pixels = 0;
		for (ui32 i = 0; i < mipLevels; ++i) pixels += widths[i] * heights[i];

		return calcByteSize(pixels, perPixelByteSize);
	}

	void Image::convertFormat(ui32 width, ui32 height, TextureFormat srcFormat, const ui8* src, TextureFormat dstFormat, ui8* dst) {
		switch (dstFormat) {
		case TextureFormat::R8G8B8A8:
		{
			switch (srcFormat) {
			case TextureFormat::R8G8B8:
				_convertFormat_R8G8B8_R8G8B8A8(width, height, src, dst);
				break;
			case TextureFormat::R8G8B8A8:
				memcpy(dst, src, calcByteSize(width, height, dstFormat));
				break;
			default:
				break;
			}
		}
		default:
			break;
		}
	}

	void Image::convertFormat(ui32 width, ui32 height, TextureFormat srcFormat, const ui8* src, ui32 mipLevels, TextureFormat dstFormat, ui8* dst) {
		auto srcPerPixelSize = calcPerPixelByteSize(srcFormat);
		auto dstPerPixelSize = calcPerPixelByteSize(dstFormat);

		convertFormat(width, height, srcFormat, src, dstFormat, dst);

		for (ui32 lv = 1; lv < mipLevels; ++lv) {
			src += calcByteSize(width, height, srcPerPixelSize);
			dst += calcByteSize(width, height, dstPerPixelSize);
			width = calcNextMipPixelSize(width);
			height = calcNextMipPixelSize(height);

			convertFormat(width, height, srcFormat, src, dstFormat, dst);
		}
	}

	void Image::_convertFormat_R8G8B8_R8G8B8A8(ui32 width, ui32 height, const ui8* src, ui8* dst) {
		ui32 numPixels = width * height;
		ui32 srcIdx = 0, dstIdx = 0;
		for (ui32 i = 0; i < numPixels; ++i) {
			memcpy(dst + dstIdx, src + srcIdx, 3);
			srcIdx += 3;
			dstIdx += 3;
			dst[dstIdx++] = 255;
		}
	}

	void Image::generateMips(TextureFormat format, ui32 mipLevels, ui8* dst) const {
		switch (format) {
		case TextureFormat::R8G8B8:
		case TextureFormat::R8G8B8A8:
		{
			convertFormat(width, height, this->format, (const ui8*)source.getBytes(), format, dst);
			generateMips_UInt8s(width, height, format, mipLevels, 4, dst + calcByteSize(width, height, format));

			break;
		}
		default:
			break;
		}
	}

	void Image::generateMips_UInt8s(ui32 width, ui32 height, modules::graphics::TextureFormat format, ui32 mipLevels, ui8 numChannels, ui8* data) {
		ui32 perPixelByteSize = calcPerPixelByteSize(format);
		ui8* src = data;
		ui8* dst = data;

		ui8 c[16];
		for (ui32 lv = 1; lv < mipLevels; ++lv) {
			dst += calcByteSize(width, height, perPixelByteSize);
			ui32 srcRowByteSize = calcByteSize(width, perPixelByteSize);
			auto w = calcNextMipPixelSize(width);
			auto h = calcNextMipPixelSize(height);

			if (w == width) {
				for (ui32 y = 0; y < h; ++y) {
					ui32 dstBegin = y * srcRowByteSize;
					auto srcRow = src + (dstBegin << 1);
					auto dstRow = dst + dstBegin;
					for (ui32 x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelByteSize;
						auto data = srcRow + (dstOffset << 1);
						memcpy(c, data, 4);
						memcpy(c, data + srcRowByteSize, 4);
						_mipPixelsBlend(c, numChannels, 2);
						memcpy(dstRow + dstOffset, c, 4);
					}
				}
			} else if (h == height) {
				for (ui32 y = 0; y < h; ++y) {
					ui32 dstBegin = y * srcRowByteSize;
					auto srcRow = src + (dstBegin << 1);
					auto dstRow = dst + dstBegin;
					for (ui32 x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelByteSize;
						auto data = srcRow + (dstOffset << 1);
						memcpy(c, data, 8);
						_mipPixelsBlend(c, numChannels, 2);
						memcpy(dstRow + dstOffset, c, 4);
					}
				}
			} else {
				for (ui32 y = 0; y < h; ++y) {
					ui32 dstBegin = y * srcRowByteSize;
					auto srcRow = src + (dstBegin << 1);
					auto dstRow = dst + dstBegin;
					for (ui32 x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelByteSize;
						auto data = srcRow + (dstOffset << 1);
						memcpy(c, data, 8);
						memcpy(c, data + srcRowByteSize, 8);
						_mipPixelsBlend(c, numChannels, 4);
						memcpy(dstRow + dstOffset, c, 4);
					}
				}
			}

			src = dst;
			width = w;
			height = h;
		}
	}

	void Image::_mipPixelsBlend(ui8* c, ui8 numChannels, ui8 numPixels) {
		auto c0 = c;
		ui16 c1[4] = { 0 };
		for (ui8 i = 0; i < numPixels; ++i) {
			for (ui8 j = 0; j < numChannels; ++j) c1[j] += c0[j];
			c0 += numChannels;
		}
		for (ui8 i = 0; i < numChannels; ++i) c[i] = (ui8)(c1[i] / numChannels);
	}
}