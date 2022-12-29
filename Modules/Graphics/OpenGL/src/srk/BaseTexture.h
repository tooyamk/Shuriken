#pragma once

#include "Base.h"
#include <unordered_set>

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL BaseTexture {
	public:
		BaseTexture(TextureType texType);
		virtual ~BaseTexture();
		
		bool SRK_CALL create(Graphics& graphics, const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount,
			TextureFormat format, Usage resUsage, const void*const* data = nullptr);
		Usage SRK_CALL map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage);
		void SRK_CALL unmap(uint32_t arraySlice, uint32_t mipSlice);
		uint32_t SRK_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen);
		uint32_t SRK_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length);
		bool SRK_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data);
		bool SRK_CALL copyFrom(Graphics& graphics, const Vec3ui32& dstPos, uint32_t dstArraySlice, uint32_t dstMipSlice, const ITextureResource* src, uint32_t srcArraySlice, uint32_t srcMipSlice, const Box3ui32& srcRange);
		bool SRK_CALL copyFrom(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer);
		void SRK_CALL flush();
		void SRK_CALL releaseTex();
		void SRK_CALL waitServerSync();
		void SRK_CALL releaseSync();

		bool dirty;
		SampleCount sampleCount;
		TextureType texType;
		TextureFormat format;
		Usage resUsage;
		Usage mapUsage;
		uint16_t perPixelSize;
		Vec3ui32 texSize;
		uint32_t arraySize;
		uint32_t internalArraySize;
		uint32_t mipLevels;

		struct {
			GLenum target;
			GLenum internalFormat;
			GLenum format;
			GLenum type;
		} glTexInfo;

		uint32_t size;
		GLuint handle;
		void* mapData;

		//GLenum internalFormat;

		GLsync sync;

	private:
		bool SRK_CALL _update(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data);
		bool SRK_CALL _createDone(Graphics& graphics, bool succeeded);
	};
}