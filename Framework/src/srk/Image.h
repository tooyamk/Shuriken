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

		modules::graphics::TextureFormat format;
		Vec2ui32 size;
		ByteArray source;

		static bool SRK_CALL convertFormat(const Vec2ui32& size, modules::graphics::TextureFormat srcFormat, const uint8_t* src, modules::graphics::TextureFormat dstFormat, uint8_t* dst);
		static bool SRK_CALL convertFormat(const Vec2ui32& size, modules::graphics::TextureFormat srcFormat, const uint8_t* src, uint32_t mipLevels, modules::graphics::TextureFormat dstFormat, uint8_t* dst);

		inline bool SRK_CALL generateMips(modules::graphics::TextureFormat format, uint32_t mipLevels, ByteArray& dst, std::vector<void*>& dstDataPtr) const {
			dst = ByteArray(calcMipsByteSize(size, mipLevels, calcPerPixelByteSize(format)));
			dstDataPtr.resize(mipLevels);
			return generateMips(format, mipLevels, (uint8_t*)dst.getSource(), dstDataPtr.data());
		}
		bool SRK_CALL generateMips(modules::graphics::TextureFormat format, uint32_t mipLevels, uint8_t* dst, void** dstDataPtr) const;

		static uint32_t SRK_CALL calcPerPixelByteSize(modules::graphics::TextureFormat format);

		inline static uint32_t SRK_CALL calcMipLevels(const Vec2ui32& size) {
			return calcMipLevels(size.getMax());
		}
		inline static uint32_t SRK_CALL calcMipLevels(uint32_t n) {
			return (uint32_t)std::floor(std::log2(n) + 1);
		}

		inline static void SRK_CALL calcSpecificMipPixelSize(uint32_t& size, uint32_t mipLevel) {
			for (uint32_t i = 0; i < mipLevel; ++i) size = calcNextMipPixelSize(size);
		}
		inline static void SRK_CALL calcSpecificMipPixelSize(Vec2ui32& size, uint32_t mipLevel) {
			for (uint32_t i = 0; i < mipLevel; ++i) calcNextMipPixelSize(size);
		}
		inline static void SRK_CALL calcSpecificMipPixelSize(Vec3ui32& size, uint32_t mipLevel) {
			for (uint32_t i = 0; i < mipLevel; ++i) calcNextMipPixelSize(size);
		}

		static std::vector<uint32_t> SRK_CALL calcMipsPixelSize(uint32_t n, uint32_t mipLevels);

		inline static uint32_t SRK_CALL calcNextMipPixelSize(uint32_t n) {
			return n > 1 ? n >> 1 : 1;
		}
		inline static void SRK_CALL calcNextMipPixelSize(Vec2ui32& size) {
			size.set(calcNextMipPixelSize(size[0]), calcNextMipPixelSize(size[1]));
		}
		inline static void SRK_CALL calcNextMipPixelSize(Vec3ui32& size) {
			size.set(calcNextMipPixelSize(size[0]), calcNextMipPixelSize(size[1]), calcNextMipPixelSize(size[2]));
		}

		inline static uint32_t SRK_CALL calcPixelsByteSize(const Vec2ui32& size, modules::graphics::TextureFormat format) {
			return calcPixelsByteSize(size.getMultiplies(), format);
		}
		inline constexpr static uint32_t SRK_CALL calcPixelsByteSize(const Vec2ui32& size, uint32_t perPixelByteSize) {
			return calcPixelsByteSize(size.getMultiplies(), perPixelByteSize);
		}
		inline static uint32_t SRK_CALL calcPixelsByteSize(uint32_t numPixels, modules::graphics::TextureFormat format) {
			return calcPixelsByteSize(numPixels, calcPerPixelByteSize(format));
		}
		inline constexpr static uint32_t SRK_CALL calcPixelsByteSize(uint32_t numPixels, uint32_t perPixelByteSize) {
			return numPixels * perPixelByteSize;
		}

		static uint32_t SRK_CALL calcMipsByteSize(const Vec2ui32& size, uint32_t mipLevels, uint32_t perPixelByteSize);
		static uint32_t SRK_CALL calcMipsByteSize(const Vec3ui32& size, uint32_t mipLevels, uint32_t perPixelByteSize);

		static void SRK_CALL generateMips_UInt8s(const Vec2ui32& size, modules::graphics::TextureFormat format, uint32_t mipLevels, uint8_t numChannels, uint8_t* dst, void** dataPtr);

		bool SRK_CALL flipY();

	private:
		static void SRK_CALL _convertFormat_R8G8B8_R8G8B8A8(const Vec2ui32& size, const uint8_t* src, uint8_t* dst);
		static void SRK_CALL _convertFormat_R8G8B8A8_R8G8B8(const Vec2ui32& size, const uint8_t* src, uint8_t* dst);

		static void SRK_CALL _mipPixelsBlend(uint8_t* c, uint8_t numChannels, uint8_t numPixels);
	};
}