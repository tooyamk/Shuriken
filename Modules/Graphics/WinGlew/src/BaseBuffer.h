#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL BaseBuffer {
	public:
		BaseBuffer(GLenum bufferType);
		virtual ~BaseBuffer();

		bool AE_CALL create(ui32 size, Usage resUsage, const void* data = nullptr);
		Usage AE_CALL map(Usage expectMapUsage);
		void AE_CALL unmap();
		i32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1);
		i32 AE_CALL write(ui32 offset, const void* data, ui32 length);
		i32 AE_CALL update(ui32 offset, const void* data, ui32 length);
		void AE_CALL flush();
		void AE_CALL releaseBuffer();
		void AE_CALL waitServerSync();
		void AE_CALL releaseSync();

		bool dirty;
		Usage resUsage;
		Usage mapUsage;
		GLenum bufferType;
		ui32 size;
		GLuint handle;
		void* mapData;

		GLsync sync;
	};
}