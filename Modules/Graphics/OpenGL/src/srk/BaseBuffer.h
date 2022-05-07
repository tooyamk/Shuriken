#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL BaseBuffer {
	public:
		BaseBuffer(GLenum bufferType);
		virtual ~BaseBuffer();

		bool SRK_CALL create(Graphics& graphics, size_t size, Usage resUsage, const void* data = nullptr, GLenum internalUsage = 0);
		Usage SRK_CALL map(Usage expectMapUsage, GLenum access = 0);
		void SRK_CALL unmap();
		size_t SRK_CALL read(size_t offset, void* dst, size_t dstLen);
		size_t SRK_CALL write(size_t offset, const void* data, size_t length);
		size_t SRK_CALL update(size_t offset, const void* data, size_t length);
		void SRK_CALL releaseBuffer();

		template<bool Force>
		void SRK_CALL doSync() {
			using namespace srk::enum_operators;

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

		inline bool SRK_CALL isSyncing() const {
			return sync ? _isSyncing() : false;
		}

		inline void SRK_CALL releaseSync() {
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
		inline void SRK_CALL _releaseSync() const {
			glDeleteSync(sync);
			sync = nullptr;
		}

		inline bool SRK_CALL _isSyncing() const {
			if (auto rst = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1); rst == GL_ALREADY_SIGNALED || rst == GL_CONDITION_SATISFIED) {
				_releaseSync();
				return false;
			}
			return true;
		}

		inline void SRK_CALL _waitServerSync() {
			if (sync) {
				do {} while (_isSyncing());
			}
		}
	};
}