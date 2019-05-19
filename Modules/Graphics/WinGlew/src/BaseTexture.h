#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL BaseTexture {
	public:
		BaseTexture(TextureType texType);
		virtual ~BaseTexture();
		
		bool AE_CALL create(Graphics& graphics, const Vec3ui32& size, ui32 arraySize, ui32 mipLevels,
			TextureFormat format, Usage resUsage, const void*const* data = nullptr);
		Usage AE_CALL map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage);
		void AE_CALL unmap(ui32 arraySlice, ui32 mipSlice);
		i32 AE_CALL read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1);
		i32 AE_CALL write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length);
		bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box3ui32& range, const void* data);
		void AE_CALL flush();
		void AE_CALL releaseTex();
		void AE_CALL waitServerSync();
		void AE_CALL releaseSync();

		bool dirty;
		TextureType texType;
		ui16 perPixelSize;
		Vec3ui32 texSize;
		ui32 arraySize;
		ui32 mipLevels;

		struct {
			GLenum target;
			GLenum internalFormat;
			GLenum format;
			GLenum type;
		} glTexInfo;

		Usage resUsage;
		Usage mapUsage;
		ui32 size;
		GLuint handle;
		void* mapData;

		//TextureFormat format;
		//GLenum internalFormat;
		

		GLsync sync;
	};
}