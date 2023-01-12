#pragma once

#include "Base.h"
#include <unordered_set>

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL BaseTexture {
	public:
		BaseTexture(TextureType texType);
		virtual ~BaseTexture();
		
		bool SRK_CALL create(Graphics& graphics, const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount,
			TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data = nullptr);
		Usage SRK_CALL map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage);
		void SRK_CALL unmap(size_t arraySlice, size_t mipSlice);
		size_t SRK_CALL read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen);
		size_t SRK_CALL write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length);
		bool SRK_CALL update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data);
		bool SRK_CALL copyFrom(Graphics& graphics, const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange);
		bool SRK_CALL copyFrom(Graphics& graphics, size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer);
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
		Vec3uz dim;
		size_t arraySize;
		size_t internalArraySize;
		size_t mipLevels;

		struct {
			GLenum target;
			GLenum internalFormat;
			GLenum format;
			GLenum type;
		} glTexInfo;

		size_t size;
		GLuint handle;
		void* mapData;

		//GLenum internalFormat;

		GLsync sync;

	private:
		bool SRK_CALL _update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data);
		bool SRK_CALL _createDone(Graphics& graphics, bool succeeded);
	};
}