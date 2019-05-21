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

		bool dirty;
		ui8 numBuffers;
		Usage resUsage;
		Usage mapUsage;
		GLenum bufferType;
		ui32 size;
		GLuint curHandle;

		union {
			struct {
				GLuint handle;
				void* mapData;
				GLsync sync;
			};
			struct {
				ui8 curIndex;
				GLuint* handles;
				void** mapDatas;
				GLsync* syncs;
			};
		} bufferData;

	private:
		void AE_CALL _createPersistentMapBuffer(GLuint handle, void*& mapData, const void* data);

		inline void*& _getCurMapData() {
			return numBuffers > 1 ? bufferData.mapDatas[bufferData.curIndex] : bufferData.mapData;
		}

		inline GLsync& _getCurSync() {
			return numBuffers > 1 ? bufferData.syncs[bufferData.curIndex] : bufferData.sync;
		}

		inline void AE_CALL _releaseSync(GLsync& sync) {
			if (sync) {
				glDeleteSync(sync);
				sync = nullptr;
			}
		}

		inline bool AE_CALL _isSyncing(GLsync& sync) {
			if (auto rst = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1); rst == GL_ALREADY_SIGNALED || rst == GL_CONDITION_SATISFIED) {
				_releaseSync(sync);
				return false;
			}
			return true;
		}

		inline void AE_CALL _waitServerSync() {
			if (auto& sync = _getCurSync(); sync) {
				do {} while (_isSyncing(sync));
			}
		}
	};
}