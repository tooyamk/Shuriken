#include "srk/Image.h"
#include "srk/ThreadUtility.h"

namespace srk {
	//using namespace modules;
	using namespace modules::graphics;

	Image::Image() :
		format(TextureFormat::UNKNOWN) {
	}

	Image::Image(Image&& other) noexcept :
		format(other.format),
		dimensions(other.dimensions),
		source(std::move(other.source))  {
	}

	Image& Image::operator=(Image&& other) noexcept {
		format = other.format;
		dimensions = other.dimensions;
		source = std::move(other.source);

		return *this;
	}

	std::vector<size_t> Image::calcMipsPixels(size_t n, size_t mipLevels) {
		if (!n) return std::vector<size_t>(0);
		if (!mipLevels) mipLevels = TextureUtility::getMipLevels(n);

		std::vector<size_t> levels(mipLevels);
		levels[0] = n;

		for (size_t i = 1; i < mipLevels; ++i) {
			n = TextureUtility::getNextMipPixels(n);
			levels[i] = n;
		}

		return levels;
	}

	bool Image::convertFormat(const Vec2uz& pixels, TextureFormat srcFormat, const void* src, TextureFormat dstFormat, void* dst, size_t* srcReadedBytes, size_t* dstWritedBytes) {
		switch (dstFormat) {
		case TextureFormat::R8G8B8_UNORM:
		{
			switch (srcFormat) {
			case TextureFormat::R8G8B8_UNORM:
			{
				auto n = TextureUtility::getBytes(srcFormat, pixels);
				memcpy(dst, src, n);
				if (srcReadedBytes) *srcReadedBytes = n;
				if (dstWritedBytes) *dstWritedBytes = n;

				return true;
			}
			case TextureFormat::R8G8B8_UNORM_SRGB:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t pos = 0;
				for (size_t i = 0; i < n; ++i) {
					d[pos] = Math::sRGB2Linear(s[pos]);
					++pos;
					d[pos] = Math::sRGB2Linear(s[pos]);
					++pos;
					d[pos] = Math::sRGB2Linear(s[pos]);
					++pos;
				}

				if (srcReadedBytes) *srcReadedBytes = pos;
				if (dstWritedBytes) *dstWritedBytes = pos;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					++sp;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM_SRGB:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = Math::sRGB2Linear(s[sp++]);
					d[dp++] = Math::sRGB2Linear(s[sp++]);
					d[dp++] = Math::sRGB2Linear(s[sp++]);
					++sp;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;
			}
			default:
				break;
			}

			break;
		}
		case TextureFormat::R8G8B8_UNORM_SRGB:
		{
			switch (srcFormat) {
			case TextureFormat::R8G8B8_UNORM:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t pos = 0;
				for (size_t i = 0; i < n; ++i) {
					d[pos] = Math::linear2sRGB(s[pos]);
					++pos;
					d[pos] = Math::linear2sRGB(s[pos]);
					++pos;
					d[pos] = Math::linear2sRGB(s[pos]);
					++pos;
				}

				if (srcReadedBytes) *srcReadedBytes = pos;
				if (dstWritedBytes) *dstWritedBytes = pos;
			}
			case TextureFormat::R8G8B8_UNORM_SRGB:
			{
				auto n = TextureUtility::getBytes(srcFormat, pixels);
				memcpy(dst, src, n);
				if (srcReadedBytes) *srcReadedBytes = n;
				if (dstWritedBytes) *dstWritedBytes = n;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = Math::linear2sRGB(s[sp++]);
					d[dp++] = Math::linear2sRGB(s[sp++]);
					d[dp++] = Math::linear2sRGB(s[sp++]);
					++sp;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM_SRGB:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					++sp;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;
			}
			default:
				break;
			}

			break;
		}
		case TextureFormat::R8G8B8A8_UNORM:
		{
			switch (srcFormat) {
			case TextureFormat::R8G8B8_UNORM:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					d[dp++] = 255;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;
			}
			case TextureFormat::R8G8B8_UNORM_SRGB:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = Math::sRGB2Linear(s[sp++]);
					d[dp++] = Math::sRGB2Linear(s[sp++]);
					d[dp++] = Math::sRGB2Linear(s[sp++]);
					d[dp++] = 255;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM:
			{
				auto n = TextureUtility::getBytes(srcFormat, pixels);
				memcpy(dst, src, n);
				if (srcReadedBytes) *srcReadedBytes = n;
				if (dstWritedBytes) *dstWritedBytes = n;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM_SRGB:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t pos = 0;
				for (size_t i = 0; i < n; ++i) {
					d[pos] = Math::sRGB2Linear(s[pos]);
					++pos;
					d[pos] = Math::sRGB2Linear(s[pos]);
					++pos;
					d[pos] = Math::sRGB2Linear(s[pos]);
					++pos;
					d[pos] = s[pos];
					++pos;
				}

				if (srcReadedBytes) *srcReadedBytes = pos;
				if (dstWritedBytes) *dstWritedBytes = pos;

				return true;
			}
			default:
				break;
			}

			break;
		}
		case TextureFormat::R8G8B8A8_UNORM_SRGB:
		{
			switch (srcFormat) {
			case TextureFormat::R8G8B8_UNORM:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = Math::linear2sRGB(s[sp++]);
					d[dp++] = Math::linear2sRGB(s[sp++]);
					d[dp++] = Math::linear2sRGB(s[sp++]);
					d[dp++] = 255;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;
			}
			case TextureFormat::R8G8B8_UNORM_SRGB:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t dp = 0, sp = 0;
				for (size_t i = 0; i < n; ++i) {
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					d[dp++] = s[sp++];
					d[dp++] = 255;
				}

				if (srcReadedBytes) *srcReadedBytes = sp;
				if (dstWritedBytes) *dstWritedBytes = dp;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM:
			{
				auto s = (const uint8_t*)src;
				auto d = (uint8_t*)dst;

				auto n = pixels.getMultiplies();
				size_t pos = 0;
				for (size_t i = 0; i < n; ++i) {
					d[pos] = Math::linear2sRGB(s[pos]);
					++pos;
					d[pos] = Math::linear2sRGB(s[pos]);
					++pos;
					d[pos] = Math::linear2sRGB(s[pos]);
					++pos;
					d[pos] = s[pos];
					++pos;
				}

				if (srcReadedBytes) *srcReadedBytes = pos;
				if (dstWritedBytes) *dstWritedBytes = pos;

				return true;
			}
			case TextureFormat::R8G8B8A8_UNORM_SRGB:
			{
				auto n = TextureUtility::getBytes(srcFormat, pixels);
				memcpy(dst, src, n);
				if (srcReadedBytes) *srcReadedBytes = n;
				if (dstWritedBytes) *dstWritedBytes = n;

				return true;
			}
			default:
				break;
			}

			break;
		}
		default:
			break;
		}

		return false;
	}

	bool Image::convertFormat(const Vec2uz& pixels, TextureFormat srcFormat, const void* src, size_t mipLevels, TextureFormat dstFormat, void* dst) {
		auto s = (const uint8_t*)src;
		auto d = (uint8_t*)dst;

		size_t srcOffsetBytes, dstOffsetBytes;
		if (!convertFormat(pixels, srcFormat, src, dstFormat, dst, &srcOffsetBytes, &dstOffsetBytes)) return false;
		s += srcOffsetBytes;
		d += dstOffsetBytes;

		for (size_t lv = 1; lv < mipLevels; ++lv) {
			if (!convertFormat(TextureUtility::getNextMipPixels(pixels), srcFormat, s, dstFormat, d, &srcOffsetBytes, &dstOffsetBytes)) return false;
			s += srcOffsetBytes;
			d += dstOffsetBytes;
		}
		return true;
	}

	bool Image::generateMips(modules::graphics::TextureFormat format, size_t mipLevels, ByteArray& dst, size_t dstOffset, std::vector<void*>& dstMipData) const {
		size_t bytes;
		TextureUtility::getMipsInfo(format, dimensions, mipLevels, &bytes);
		auto n = bytes + dstOffset;

		if (dst.getCapacity() < n) dst.setCapacity(n);
		dst.setLength(n);

		dstMipData.resize(mipLevels);
		return generateMips(format, mipLevels, dst.getSource() + dstOffset, dstMipData.data());
	}

	bool Image::generateMips(TextureFormat format, size_t mipLevels, void* dst, void** dstMipData) const {
		switch (format) {
		case TextureFormat::R8G8B8_UNORM:
		case TextureFormat::R8G8B8_UNORM_SRGB:
		{
			if (convertFormat(dimensions, this->format, (const uint8_t*)source.getSource(), format, dst)) {
				generateMips_UInt8s(dimensions, format, mipLevels, 3, dst, dstMipData);
				return true;
			}

			break;
		}
		case TextureFormat::R8G8B8A8_UNORM:
		case TextureFormat::R8G8B8A8_UNORM_SRGB:
		{
			if (convertFormat(dimensions, this->format, (const uint8_t*)source.getSource(), format, dst)) {
				generateMips_UInt8s(dimensions, format, mipLevels, 4, dst, dstMipData);
				return true;
			}

			break;
		}
		default:
			break;
		}

		return false;
	}

	void Image::generateMips_UInt8s(const Vec2uz& pixels, modules::graphics::TextureFormat format, size_t mipLevels, size_t numChannels, void* data, void** dstMipData) {
		uint32_t numChannels2 = numChannels << 1;
		auto perPixelBytes = TextureUtility::getPerBlockBytes(format);
		auto src = (const uint8_t*)data;
		auto dst = (uint8_t*)data;
		auto s = pixels;

		if (dstMipData) dstMipData[0] = dst;

		uint8_t c[16];
		for (size_t lv = 1; lv < mipLevels; ++lv) {
			dst += TextureUtility::getBytes(perPixelBytes, s.getMultiplies());
			if (dstMipData) dstMipData[lv] = dst;
			auto w = TextureUtility::getNextMipPixels(s[0]);
			auto h = TextureUtility::getNextMipPixels(s[1]);
			auto srcRowBytes = TextureUtility::getBytes(perPixelBytes, s[0]);
			auto dstRowBytes = TextureUtility::getBytes(perPixelBytes, w);

			if (w == s[0]) {
				if (h == s[1]) {
					memcpy(dst, src, w * h * perPixelBytes);
				} else {
					for (size_t y = 0; y < h; ++y) {
						auto dstBegin = y * dstRowBytes;
						auto srcRow = src + (dstBegin << 1);
						auto dstRow = dst + dstBegin;
						for (size_t x = 0; x < w; ++x) {
							auto dstOffset = x * perPixelBytes;
							auto data = srcRow + dstOffset;
							memcpy(c, data, numChannels);
							memcpy(c + numChannels, data + dstRowBytes, numChannels);
							_mipPixelsBlend(c, numChannels, 2);
							memcpy(dstRow + dstOffset, c, numChannels);
						}
					}
				}
			} else if (h == s[1]) {
				for (size_t y = 0; y < h; ++y) {
					auto srcRow = src + y * srcRowBytes;
					auto dstRow = dst + y * dstRowBytes;
					for (size_t x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelBytes;
						memcpy(c, srcRow + (dstOffset << 1), numChannels2);
						_mipPixelsBlend(c, numChannels, 2);
						memcpy(dstRow + dstOffset, c, numChannels);
					}
				}
			} else {
				for (size_t y = 0; y < h; ++y) {
					auto srcRow = src + (y << 1) * srcRowBytes;
					auto dstRow = dst + y * dstRowBytes;
					for (size_t x = 0; x < w; ++x) {
						auto dstOffset = x * perPixelBytes;
						auto data = srcRow + (dstOffset << 1);
						memcpy(c, data, numChannels2);
						memcpy(c + numChannels2, data + srcRowBytes, numChannels2);
						_mipPixelsBlend(c, numChannels, 4);
						memcpy(dstRow + dstOffset, c, numChannels);
					}
				}
			}

			src = dst;
			s.set(w, h);
		}
	}

	bool Image::flipY() {
		if (format == TextureFormat::UNKNOWN || TextureUtility::isCompressedFormat(format)) return false;

		auto perPixelBytes = TextureUtility::getPerBlockBytes(format);
		if (!perPixelBytes) return false;

		auto data = source.getSource();
		size_t bytesPreRow = perPixelBytes * dimensions[0];
		auto n = dimensions[1] >> 1;
		decltype(n) top = 0, bottom = dimensions[1] - 1;
		for (decltype(n) i = 0; i < n; ++i) {
			auto topBegin = top * bytesPreRow;
			auto bottomBegin = bottom * bytesPreRow;

			for (size_t j = 0; j < bytesPreRow; ++j) {
				auto val = data[topBegin + j];
				data[topBegin + j] = data[bottomBegin + j];
				data[bottomBegin + j] = val;
			}

			++top;
			--bottom;
		}

		return true;
	}

	bool Image::scale(Image& dst) const {
		if (format == TextureFormat::UNKNOWN || TextureUtility::isCompressedFormat(format)) return false;

		auto perPixelBytes = TextureUtility::getPerBlockBytes(format);
		if (!perPixelBytes) return false;

		auto dstBytes = dst.dimensions.getMultiplies() * perPixelBytes;
		dst.format = format;
		dst.source = std::move(ByteArray(dstBytes, dstBytes));

		auto srcData = source.getSource();
		auto dstData = dst.source.getSource();

		Vec2f32 max(dimensions - 1);
		auto sd = Vec2f32(dimensions) / dst.dimensions;
		auto step = 0.5f * sd - 0.5f;

		auto nx = dst.dimensions[0], ny = dst.dimensions[1];

		for (auto y = 0u; y < ny; ++y) {
			auto v = std::clamp(y * sd[1] + step[1], 0.f, max[1]);
			auto iy = (uint32_t)v;
			v -= iy;

			auto edgeY = iy + 1 < dimensions[1] ? 1u : 0u;
			auto off1 = iy * dimensions[0];
			auto off2 = edgeY * dimensions[0];

			auto dstPosBeg = y * dst.dimensions[0];

			for (auto x = 0u; x < nx; ++x) {
				auto u = std::clamp(x * sd[0] + step[0], 0.f, max[0]);
				auto ix = (uint32_t)u;
				u -= ix;

				auto edgeX = ix + 1 < dimensions[0] ? 1u : 0u;

				auto lt = off1 + ix;
				auto rt = lt + edgeX;
				auto lb = lt + off2;
				auto rb = lb + edgeX;

				lt *= perPixelBytes;
				rt *= perPixelBytes;
				lb *= perPixelBytes;
				rb *= perPixelBytes;

				auto dstPos = (dstPosBeg + x) * perPixelBytes;

				for (auto i = 0u; i < perPixelBytes; ++i) {
					auto tc = std::lerp((float32_t)srcData[lt + i], (float32_t)srcData[rt + i], u);
					auto bc = std::lerp((float32_t)srcData[lb + i], (float32_t)srcData[rb + i], u);

					dstData[dstPos + i] = std::lerp(tc, bc, v);
				}
			}
		}

		return true;
	}

	bool Image::scale(Image& dst, size_t jobCount, size_t threadCount, size_t threadIndex) const {
		if (format != dst.format || format == TextureFormat::UNKNOWN || TextureUtility::isCompressedFormat(format)) return false;

		auto perPixelBytes = TextureUtility::getPerBlockBytes(format);
		if (!perPixelBytes || dst.source.getLength() < perPixelBytes * dst.dimensions.getMultiplies()) return false;

		auto range = ThreadUtility::calcJobRange(jobCount, threadCount, threadIndex);
		auto i = range.begin;
		auto n = i + range.count;

		auto srcData = source.getSource();
		auto dstData = dst.source.getSource();

		Vec2f32 max(dimensions - 1);
		auto sd = Vec2f32(dimensions) / dst.dimensions;
		auto step = 0.5f * sd - 0.5f;

		auto nx = dst.dimensions[0], ny = dst.dimensions[1];

		while (i < n) {
			auto div = std::div((std::make_signed_t<decltype(i)>)i, (std::make_signed_t<decltype(i)>)dst.dimensions[0]);
			auto y = div.quot;
			auto x = div.rem;

			auto v = std::clamp(y * sd[1] + step[1], 0.f, max[1]);
			auto iy = (uint32_t)v;
			v -= iy;

			auto edgeY = iy + 1 < dimensions[1] ? 1u : 0u;
			auto off1 = iy * dimensions[0];
			auto off2 = edgeY * dimensions[0];

			auto dstPosBeg = y * dst.dimensions[0];

			//
			auto u = std::clamp(x * sd[0] + step[0], 0.f, max[0]);
			auto ix = (uint32_t)u;
			u -= ix;

			auto edgeX = ix + 1 < dimensions[0] ? 1u : 0u;

			auto lt = off1 + ix;
			auto rt = lt + edgeX;
			auto lb = lt + off2;
			auto rb = lb + edgeX;

			lt *= perPixelBytes;
			rt *= perPixelBytes;
			lb *= perPixelBytes;
			rb *= perPixelBytes;

			auto dstPos = (dstPosBeg + x) * perPixelBytes;

			for (auto i = 0u; i < perPixelBytes; ++i) {
				auto tc = std::lerp((float32_t)srcData[lt + i], (float32_t)srcData[rt + i], u);
				auto bc = std::lerp((float32_t)srcData[lb + i], (float32_t)srcData[rb + i], u);

				dstData[dstPos + i] = std::lerp(tc, bc, v);
			}

			++i;
		}

		return true;
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