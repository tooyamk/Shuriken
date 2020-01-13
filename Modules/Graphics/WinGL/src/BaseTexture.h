#pragma once

#include "Base.h"
#include <unordered_set>

namespace aurora::modules::graphics::win_gl {
	class Graphics;
	class TextureView;

	class AE_MODULE_DLL BaseTexture {
	public:
		BaseTexture(TextureType texType);
		virtual ~BaseTexture();
		
		bool AE_CALL create(Graphics& graphics, const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels,
			TextureFormat format, Usage resUsage, const void*const* data = nullptr);
		Usage AE_CALL map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage);
		void AE_CALL unmap(uint32_t arraySlice, uint32_t mipSlice);
		uint32_t AE_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen);
		uint32_t AE_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length);
		bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data);
		bool AE_CALL copyFrom(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer);
		void AE_CALL flush();
		void AE_CALL releaseTex();
		void AE_CALL addView(TextureView& view);
		void AE_CALL removeView(TextureView& view);
		void AE_CALL waitServerSync();
		void AE_CALL releaseSync();

		bool dirty;
		bool isArray;
		TextureType texType;
		Usage resUsage;
		Usage mapUsage;
		uint16_t perPixelSize;
		Vec3ui32 texSize;
		uint32_t arraySize;
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

		//TextureFormat format;
		//GLenum internalFormat;
		std::unordered_set<TextureView*> views;

		GLsync sync;

	private:
		bool AE_CALL _update (uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data);
		bool AE_CALL _createDone(bool succeeded);
	};
}