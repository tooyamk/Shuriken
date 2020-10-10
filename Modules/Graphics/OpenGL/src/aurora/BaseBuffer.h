#pragma once

#include "Base.h"

namespace aurora::modules::graphics::gl {
	class Graphics;

	class AE_MODULE_DLL BaseBuffer {
	public:
		BaseBuffer(GLenum bufferType);
		virtual ~BaseBuffer();

		bool AE_CALL create(Graphics& graphics, size_t size, Usage resUsage, const void* data = nullptr, GLenum internalUsage = 0);
		Usage AE_CALL map(Usage expectMapUsage, GLenum access = 0);
		void AE_CALL unmap();
		size_t AE_CALL read(size_t offset, void* dst, size_t dstLen);
		size_t AE_CALL write(size_t offset, const void* data, size_t length);
		size_t AE_CALL update(size_t offset, const void* data, size_t length);
		void AE_CALL releaseBuffer();

		template<bool Force>
		void AE_CALL doSync() {
			if constexpr (Force) {
				releaseSync();

				if ((resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
					sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
				}
				dirty = false;
			} else {
				if (dirty) {
					if ((resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
						releaseSync();
						sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
					}
					dirty = false;
				}
			}
		}

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
		size_t size;

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