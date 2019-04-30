#include "Image.h"
#include "modules/IGraphicsModule.h"

namespace aurora {
	//using namespace modules;
	using namespace modules::graphics;

	Image::Image() :
		format(TextureFormat::UNKNOWN) {
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

	ui32 Image::calcMipsByteSize(const Vec2ui32& size, ui32 mipLevels, ui32 perPixelByteSize) {
		auto w = size[0], h = size[1];
		auto pixels = w * h;

		for (ui32 i = 1; i < mipLevels; ++i) {
			w = calcNextMipPixelSize(w);
			h = calcNextMipPixelSize(h);
			pixels += w * h;
		}

		return calcByteSize(pixels, perPixelByteSize);
	}

	ui32 Image::calcMipsByteSize(const Vec3ui32& size, ui32 mipLevels, ui32 perPixelByteSize) {
		auto w = size[0], h = size[1], d = size[2];
		auto pixels = w * h * d;

		for (ui32 i = 1; i < mipLevels; ++i) {
			w = calcNextMipPixelSize(w);
			h = calcNextMipPixelSize(h);
			d = calcNextMipPixelSize(d);
			pixels += w * h * d;
		}

		return calcByteSize(pixels, perPixelByteSize);
	}

	bool Image::convertFormat(const Vec2ui32& size, TextureFormat srcFormat, const ui8* src, TextureFormat dstFormat, ui8* dst) {
		switch (dstFormat) {
		case TextureFormat::R8G8B8:
		{
			switch (srcFormat) {
			case TextureFormat::R8G8B8:
				memcpy(dst, src, calcByteSize(size, dstFormat));
				return true;
			case TextureFormat::R8G8B8A8:
				_convertFormat_R8G8B8A8_R8G8B8(size, src, dst);
				return true;
			default:
				break;
			}
		}
		case TextureFormat::R8G8B8A8:
		{
			switch (srcFormat) {
			case TextureFormat::R8G8B8:
				_convertFormat_R8G8B8_R8G8B8A8(size, src, dst);
				return true;
			case TextureFormat::R8G8B8A8:
				memcpy(dst, src, calcByteSize(size, dstFormat));
				return true;
			default:
				break;
			}
		}
		default:
			break;
		}

		return false;
	}

	bool Image::convertFormat(const Vec2ui32& size, TextureFormat srcFormat, const ui8* src, ui32 mipLevels, TextureFormat dstFormat, ui8* dst) {
		auto srcPerPixelSize = calcPerPixelByteSize(srcFormat);
		auto dstPerPixelSize = calcPerPixelByteSize(dstFormat);

		if (!convertFormat(size, srcFormat, src, dstFormat, dst)) return false;

		auto s = size;

		for (ui32 lv = 1; lv < mipLevels; ++lv) {
			src += calcByteSize(s, srcPerPixelSize);
			dst += calcByteSize(s, dstPerPixelSize);
			s[0] = calcNextMipPixelSize(s[0]);
			s[1] = calcNextMipPixelSize(s[1]);

			if (!convertFormat(s, srcFormat, src, dstFormat, dst)) return false;
		}
		return true;
	}

	void Image::_convertFormat_R8G8B8_R8G8B8A8(const Vec2ui32& size, const ui8* src, ui8* dst) {
		auto numPixels = size.getCumprod();
		ui32 srcIdx = 0, dstIdx = 0;
		for (ui32 i = 0; i < numPixels; ++i) {
			memcpy(dst + dstIdx, src + srcIdx, 3);
			srcIdx += 3;
			dstIdx += 3;
			dst[dstIdx++] = 255;
		}
	}

	void Image::_convertFormat_R8G8B8A8_R8G8B8(const Vec2ui32& size, const ui8* src, ui8* dst) {
		auto numPixels = size.getCumprod();
		ui32 srcIdx = 0, dstIdx = 0;
		for (ui32 i = 0; i < numPixels; ++i) {
			memcpy(dst + dstIdx, src + srcIdx, 3);
			srcIdx += 4;
			dstIdx += 3;
		}
	}

	bool Image::generateMips(TextureFormat format, ui32 mipLevels, ui8* dst, void** dstDataPtr) const {
		switch (format) {
		case TextureFormat::R8G8B8:
		{
			if (convertFormat(size, this->format, (const ui8*)source.getBytes(), format, dst)) {
				generateMips_UInt8s(size, format, mipLevels, 3, dst, dstDataPtr);
				return true;
			}

			break;
		}
		case TextureFormat::R8G8B8A8:
		{
			if (convertFormat(size, this->format, (const ui8*)source.getBytes(), format, dst)) {
				generateMips_UInt8s(size, format, mipLevels, 4, dst, dstDataPtr);
				return true;
			}

			break;
		}
		default:
			break;
		}

		return false;
	}

	void Image::generateMips_UInt8s(const Vec2ui32& size, modules::graphics::TextureFormat format, ui32 mipLevels, ui8 numChannels, ui8* data, void** dataPtr) {
		ui32 numChannels2 = numChannels << 1;
		auto perPixelByteSize = calcPerPixelByteSize(format);
		auto src = data;
		auto dst = data;
		auto s = size;

		if (dataPtr) dataPtr[0] = dst;

		ui8 c[16];
		for (ui32 lv = 1; lv < mipLevels; ++lv) {
			dst += calcByteSize(s, perPixelByteSize);
			if (dataPtr) dataPtr[lv] = dst;
			auto w = calcNextMipPixelSize(s[0]);
			auto h = calcNextMipPixelSize(s[1]);
			ui32 srcRowByteSize = calcByteSize(s[0], perPixelByteSize);
			ui32 dstRowByteSize = calcByteSize(w, perPixelByteSize);

			if (w == s[0]) {
				if (h == s[1]) {
					memcpy(dst, src, w * h * perPixelByteSize);
				} else {
					for (ui32 y = 0; y < h; ++y) {
						auto dstBegin = y * dstRowByteSize;
						auto srcRow = src + (dstBegin << 1);
						auto dstRow = dst + dstBegin;
						for (ui32 x = 0; x < w; ++x) {
							auto dstOffset = x * perPixelByteSize;
							auto data = srcRow + dstOffset;
							memcpy(c, data, numChannels);
							memcpy(c + numChannels, data + dstRowByteSize, numChannels);
							_mipPixelsBlend(c, numChannels, 2);
							memcpy(dstRow + dstOffset, c, numChannels);
						}
					}
				}
			} else if (h == s[1]) {
				for (ui32 y = 0; y < h; ++y) {
					auto srcRow = src + y * srcRowByteSize;
					auto dstRow = dst + y * dstRowByteSize;
					for (ui32 x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelByteSize;
						memcpy(c, srcRow + (dstOffset << 1), numChannels2);
						_mipPixelsBlend(c, numChannels, 2);
						memcpy(dstRow + dstOffset, c, numChannels);
					}
				}
			} else {
				for (ui32 y = 0; y < h; ++y) {
					auto srcRow = src + (y << 1) * srcRowByteSize;
					auto dstRow = dst + y * dstRowByteSize;
					for (ui32 x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelByteSize;
						auto data = srcRow + (dstOffset << 1);
						memcpy(c, data, numChannels2);
						memcpy(c + numChannels2, data + srcRowByteSize, numChannels2);
						_mipPixelsBlend(c, numChannels, 4);
						memcpy(dstRow + dstOffset, c, numChannels);
					}
				}
			}

			src = dst;
			s[0] = w;
			s[1] = h;
		}
	}

	void Image::_mipPixelsBlend(ui8* c, ui8 numChannels, ui8 numPixels) {
		auto c0 = c;
		ui16 c1[4] = { 0 };
		for (ui8 i = 0; i < numPixels; ++i) {
			for (ui8 j = 0; j < numChannels; ++j) c1[j] += c0[j];
			c0 += numChannels;
		}
		for (ui8 i = 0; i < numChannels; ++i) c[i] = (ui8)(c1[i] / numPixels);
	}
}