#pragma once

#include "base/ByteArray.h"
#include "base/Ref.h"
#include "math/Vector.h"
#include <vector>

namespace aurora {
	namespace modules::graphics {
		enum class TextureFormat : uint8_t;
	}

	class AE_DLL Image : public Ref {
	public:
		Image();

		modules::graphics::TextureFormat format;
		Vec2ui32 size;
		ByteArray source;

		static bool convertFormat(const Vec2ui32& size, modules::graphics::TextureFormat srcFormat, const uint8_t* src, modules::graphics::TextureFormat dstFormat, uint8_t* dst);
		static bool convertFormat(const Vec2ui32& size, modules::graphics::TextureFormat srcFormat, const uint8_t* src, uint32_t mipLevels, modules::graphics::TextureFormat dstFormat, uint8_t* dst);

		inline bool generateMips(modules::graphics::TextureFormat format, uint32_t mipLevels, ByteArray& dst, std::vector<void*>& dstDataPtr) const {
			dst = ByteArray(calcMipsByteSize(size, mipLevels, calcPerPixelByteSize(format)));
			dstDataPtr.resize(mipLevels);
			return generateMips(format, mipLevels, (uint8_t*)dst.getSource(), dstDataPtr.data());
		}
		bool generateMips(modules::graphics::TextureFormat format, uint32_t mipLevels, uint8_t* dst, void** dstDataPtr) const;

		static uint32_t calcPerPixelByteSize(modules::graphics::TextureFormat format);

		inline static uint32_t AE_CALL calcMipLevels(const Vec2ui32& size) {
			return calcMipLevels(size.getMax());
		}
		inline static uint32_t AE_CALL calcMipLevels(uint32_t n) {
			return (uint32_t)std::floor(std::log2(n) + 1);
		}

		inline static void AE_CALL calcSpecificMipPixelSize(uint32_t& size, uint32_t mipLevel) {
			for (uint32_t i = 0; i < mipLevel; ++i) size = calcNextMipPixelSize(size);
		}
		inline static void AE_CALL calcSpecificMipPixelSize(Vec2ui32& size, uint32_t mipLevel) {
			for (uint32_t i = 0; i < mipLevel; ++i) calcNextMipPixelSize(size);
		}
		inline static void AE_CALL calcSpecificMipPixelSize(Vec3ui32& size, uint32_t mipLevel) {
			for (uint32_t i = 0; i < mipLevel; ++i) calcNextMipPixelSize(size);
		}

		static std::vector<uint32_t> AE_CALL calcMipsPixelSize(uint32_t n, uint32_t mipLevels);

		inline static uint32_t calcNextMipPixelSize(uint32_t n) {
			return n > 1 ? n >> 1 : 1;
		}
		inline static void calcNextMipPixelSize(Vec2ui32& size) {
			size.set(calcNextMipPixelSize(size[0]), calcNextMipPixelSize(size[1]));
		}
		inline static void calcNextMipPixelSize(Vec3ui32& size) {
			size.set(calcNextMipPixelSize(size[0]), calcNextMipPixelSize(size[1]), calcNextMipPixelSize(size[2]));
		}

		inline static uint32_t calcByteSize(const Vec2ui32& size, modules::graphics::TextureFormat format) {
			return calcByteSize(size.getMultiplies(), format);
		}
		inline constexpr static uint32_t calcByteSize(const Vec2ui32& size, uint32_t perPixelByteSize) {
			return calcByteSize(size.getMultiplies(), perPixelByteSize);
		}
		inline static uint32_t calcByteSize(uint32_t numPixels, modules::graphics::TextureFormat format) {
			return calcByteSize(numPixels, calcPerPixelByteSize(format));
		}
		inline constexpr static uint32_t calcByteSize(uint32_t numPixels, uint32_t perPixelByteSize) {
			return numPixels * perPixelByteSize;
		}

		static uint32_t calcMipsByteSize(const Vec2ui32& size, uint32_t mipLevels, uint32_t perPixelByteSize);
		static uint32_t calcMipsByteSize(const Vec3ui32& size, uint32_t mipLevels, uint32_t perPixelByteSize);

		static void generateMips_UInt8s(const Vec2ui32& size, modules::graphics::TextureFormat format, uint32_t mipLevels, uint8_t numChannels, uint8_t* dst, void** dataPtr);

	private:
		static void _convertFormat_R8G8B8_R8G8B8A8(const Vec2ui32& size, const uint8_t* src, uint8_t* dst);
		static void _convertFormat_R8G8B8A8_R8G8B8(const Vec2ui32& size, const uint8_t* src, uint8_t* dst);

		static void _mipPixelsBlend(uint8_t* c, uint8_t numChannels, uint8_t numPixels);
	};
}