#pragma once

#include "Base.h"
#include "Graphics.h"

namespace srk::modules::graphics::gl {
	class SRK_MODULE_DLL BaseBuffer {
	public:
		BaseBuffer(GLenum bufferType);
		virtual ~BaseBuffer();

		bool SRK_CALL create(Graphics& graphics, size_t size, Usage requiredUsage, Usage preferredUsage, const void* data = nullptr, GLenum internalUsage = 0);
		Usage SRK_CALL map(Usage expectMapUsage, GLenum access = 0);
		void SRK_CALL unmap();
		size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset);
		size_t SRK_CALL write(const void* data, size_t length, size_t offset);
		size_t SRK_CALL update(const void* data, size_t length, size_t offset);
		void SRK_CALL releaseBuffer();

		size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange);
		size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange);

		template<bool Force>
		inline void SRK_CALL doSync() {
			using namespace srk::enum_operators;

			if constexpr (Force) {
				_doSync();
			} else {
				if (mapDirty) _doSync();
			}
		}

		inline bool SRK_CALL isSyncing() const {
			return sync ? _isSyncing() : false;
		}

		inline void SRK_CALL releaseSync() {
			if (sync) _releaseSync();
		}

		bool mapDirty;
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

		inline void SRK_CALL _waitServerSync() const {
			if (sync) {
				while (_isSyncing()) {};
			}
		}

		void SRK_CALL _doSync() {
			using namespace srk::enum_operators;

			if ((resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
				releaseSync();
				sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			}
			mapDirty = false;
		}
	};
}