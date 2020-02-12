#include "aurora/Image.h"
#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora {
	//using namespace modules;
	using namespace modules::graphics;

	Image::Image() :
		format(TextureFormat::UNKNOWN) {
	}

	uint32_t Image::calcPerPixelByteSize(TextureFormat format) {
		switch (format) {
		case TextureFormat::R8G8B8:
			return 3;
		case TextureFormat::R8G8B8A8:
			return 4;
		default:
			return 0;
		}
	}

	std::vector<uint32_t> Image::calcMipsPixelSize(uint32_t n, uint32_t mipLevels) {
		if (!n) return std::move(std::vector<uint32_t>(0));
		if (!mipLevels) mipLevels = calcMipLevels(n);

		std::vector<uint32_t> levels(mipLevels);
		levels[0] = n;

		for (uint32_t i = 1; i < mipLevels; ++i) {
			n = calcNextMipPixelSize(n);
			levels[i] = n;
		}

		return std::move(levels);
	}

	uint32_t Image::calcMipsByteSize(const Vec2ui32& size, uint32_t mipLevels, uint32_t perPixelByteSize) {
		auto w = size[0], h = size[1];
		auto pixels = w * h;

		for (uint32_t i = 1; i < mipLevels; ++i) {
			w = calcNextMipPixelSize(w);
			h = calcNextMipPixelSize(h);
			pixels += w * h;
		}

		return calcByteSize(pixels, perPixelByteSize);
	}

	uint32_t Image::calcMipsByteSize(const Vec3ui32& size, uint32_t mipLevels, uint32_t perPixelByteSize) {
		auto w = size[0], h = size[1], d = size[2];
		auto pixels = w * h * d;

		for (uint32_t i = 1; i < mipLevels; ++i) {
			w = calcNextMipPixelSize(w);
			h = calcNextMipPixelSize(h);
			d = calcNextMipPixelSize(d);
			pixels += w * h * d;
		}

		return calcByteSize(pixels, perPixelByteSize);
	}

	bool Image::convertFormat(const Vec2ui32& size, TextureFormat srcFormat, const uint8_t* src, TextureFormat dstFormat, uint8_t* dst) {
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

	bool Image::convertFormat(const Vec2ui32& size, TextureFormat srcFormat, const uint8_t* src, uint32_t mipLevels, TextureFormat dstFormat, uint8_t* dst) {
		auto srcPerPixelSize = calcPerPixelByteSize(srcFormat);
		auto dstPerPixelSize = calcPerPixelByteSize(dstFormat);

		if (!convertFormat(size, srcFormat, src, dstFormat, dst)) return false;

		auto s = size;

		for (uint32_t lv = 1; lv < mipLevels; ++lv) {
			src += calcByteSize(s, srcPerPixelSize);
			dst += calcByteSize(s, dstPerPixelSize);
			s.set(calcNextMipPixelSize(s[0]), calcNextMipPixelSize(s[1]));

			if (!convertFormat(s, srcFormat, src, dstFormat, dst)) return false;
		}
		return true;
	}

	void Image::_convertFormat_R8G8B8_R8G8B8A8(const Vec2ui32& size, const uint8_t* src, uint8_t* dst) {
		auto numPixels = size.getMultiplies();
		uint32_t srcIdx = 0, dstIdx = 0;
		for (uint32_t i = 0; i < numPixels; ++i) {
			memcpy(dst + dstIdx, src + srcIdx, 3);
			srcIdx += 3;
			dstIdx += 3;
			dst[dstIdx++] = 255;
		}
	}

	void Image::_convertFormat_R8G8B8A8_R8G8B8(const Vec2ui32& size, const uint8_t* src, uint8_t* dst) {
		auto numPixels = size.getMultiplies();
		uint32_t srcIdx = 0, dstIdx = 0;
		for (uint32_t i = 0; i < numPixels; ++i) {
			memcpy(dst + dstIdx, src + srcIdx, 3);
			srcIdx += 4;
			dstIdx += 3;
		}
	}

	bool Image::generateMips(TextureFormat format, uint32_t mipLevels, uint8_t* dst, void** dstDataPtr) const {
		switch (format) {
		case TextureFormat::R8G8B8:
		{
			if (convertFormat(size, this->format, (const uint8_t*)source.getSource(), format, dst)) {
				generateMips_UInt8s(size, format, mipLevels, 3, dst, dstDataPtr);
				return true;
			}

			break;
		}
		case TextureFormat::R8G8B8A8:
		{
			if (convertFormat(size, this->format, (const uint8_t*)source.getSource(), format, dst)) {
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

	void Image::generateMips_UInt8s(const Vec2ui32& size, modules::graphics::TextureFormat format, uint32_t mipLevels, uint8_t numChannels, uint8_t* data, void** dataPtr) {
		uint32_t numChannels2 = numChannels << 1;
		auto perPixelByteSize = calcPerPixelByteSize(format);
		auto src = data;
		auto dst = data;
		auto s = size;

		if (dataPtr) dataPtr[0] = dst;

		uint8_t c[16];
		for (uint32_t lv = 1; lv < mipLevels; ++lv) {
			dst += calcByteSize(s, perPixelByteSize);
			if (dataPtr) dataPtr[lv] = dst;
			auto w = calcNextMipPixelSize(s[0]);
			auto h = calcNextMipPixelSize(s[1]);
			uint32_t srcRowByteSize = calcByteSize(s[0], perPixelByteSize);
			uint32_t dstRowByteSize = calcByteSize(w, perPixelByteSize);

			if (w == s[0]) {
				if (h == s[1]) {
					memcpy(dst, src, w * h * perPixelByteSize);
				} else {
					for (uint32_t y = 0; y < h; ++y) {
						auto dstBegin = y * dstRowByteSize;
						auto srcRow = src + (dstBegin << 1);
						auto dstRow = dst + dstBegin;
						for (uint32_t x = 0; x < w; ++x) {
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
				for (uint32_t y = 0; y < h; ++y) {
					auto srcRow = src + y * srcRowByteSize;
					auto dstRow = dst + y * dstRowByteSize;
					for (uint32_t x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelByteSize;
						memcpy(c, srcRow + (dstOffset << 1), numChannels2);
						_mipPixelsBlend(c, numChannels, 2);
						memcpy(dstRow + dstOffset, c, numChannels);
					}
				}
			} else {
				for (uint32_t y = 0; y < h; ++y) {
					auto srcRow = src + (y << 1) * srcRowByteSize;
					auto dstRow = dst + y * dstRowByteSize;
					for (uint32_t x = 0; x < w; ++x) {
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
			s.set(w, h);
		}
	}

	void Image::_mipPixelsBlend(uint8_t* c, uint8_t numChannels, uint8_t numPixels) {
		auto c0 = c;
		uint8_t c1[4] = { 0 };
		for (uint8_t i = 0; i < numPixels; ++i) {
			for (uint8_t j = 0; j < numChannels; ++j) c1[j] += c0[j];
			c0 += numChannels;
		}
		for (uint8_t i = 0; i < numChannels; ++i) c[i] = (uint8_t)(c1[i] / numPixels);
	}
}