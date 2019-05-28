#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL BaseBuffer {
	public:
		BaseBuffer(GLenum bufferType);
		virtual ~BaseBuffer();

		bool AE_CALL create(Graphics& graphics, ui32 size, Usage resUsage, const void* data = nullptr);
		Usage AE_CALL map(Usage expectMapUsage);
		void AE_CALL unmap();
		ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen);
		ui32 AE_CALL write(ui32 offset, const void* data, ui32 length);
		ui32 AE_CALL update(ui32 offset, const void* data, ui32 length);
		void AE_CALL flush();
		void AE_CALL releaseBuffer();

		inline bool AE_CALL isSyncing() const {
			return sync ? _isSyncing() : false;
		}

		inline void AE_CALL releaseSync() {
			if (sync) _releaseSync();
		}

		bool dirty;
		Usage resUsage;
		Usage mapUsage;
		GLenum bufferType;
		ui32 size;

		GLuint handle;
		void* mapData;
		mutable GLsync sync;

	private:
		inline void AE_CALL _releaseSync() const {
			glDeleteSync(sync);
			sync = nullptr;
		}

		inline bool AE_CALL _isSyncing() const {
			if (auto rst = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1); rst == GL_ALREADY_SIGNALED || rst == GL_CONDITION_SATISFIED) {
				_releaseSync();
				return false;
			}
			return true;
		}

		inline void AE_CALL _waitServerSync() {
			if (sync) {
				do {} while (_isSyncing());
			}
		}
	};
}