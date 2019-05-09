#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_glew {
	BaseBuffer::BaseBuffer(GLenum bufferType) :
		dirty(false),
		resUsage(Usage::NONE),
		mapUsage(Usage::NONE),
		bufferType(bufferType),
		size(0),
		handle(0),
		mapData(nullptr),
		sync(nullptr) {
	}

	BaseBuffer::~BaseBuffer() {
		releaseBuffer();
	}

	bool BaseBuffer::create(Graphics* graphics, ui32 size, Usage resUsage, const void* data) {
		releaseBuffer();

		glGenBuffers(1, &handle);

		if (handle) {
			this->resUsage = resUsage & (Usage::CPU_READ_WRITE | Usage::GPU_WRITE);
			this->size = size;

			glBindBuffer(bufferType, handle);

			if ((resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {

			}

			GLbitfield flags = GL_MAP_WRITE_BIT
				| GL_MAP_PERSISTENT_BIT //在被映射状态下不同步
				| GL_MAP_COHERENT_BIT;  //数据对GPU立即可见

			
			//glBufferData(bufferType, size, data, GL_DYNAMIC_DRAW);
			glBufferStorage(bufferType, size, data, flags);
			mapData = glMapBufferRange(bufferType, 0, size, flags);

			return true;
		}

		releaseBuffer();
		return false;
	}

	Usage BaseBuffer::map(Usage expectMapUsage) {
		Usage ret = Usage::NONE;

		expectMapUsage &= Usage::CPU_READ_WRITE;
		if (handle && expectMapUsage != Usage::NONE) {
			if (((expectMapUsage & Usage::CPU_READ) == Usage::CPU_READ) && ((resUsage & Usage::CPU_READ) == Usage::CPU_READ)) {
				ret |= Usage::CPU_READ;
			} else {
				expectMapUsage &= ~Usage::CPU_READ;
			}
			if ((expectMapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
				if ((resUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
					ret |= Usage::CPU_WRITE;
				}
			} else {
				expectMapUsage &= ~Usage::CPU_WRITE;
			}

			mapUsage = expectMapUsage;
		}

		return ret;
	}

	void BaseBuffer::unmap() {
		if (mapUsage != Usage::NONE) {
			mapUsage = Usage::NONE;

			flush();
		}
	}

	i32 BaseBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return -1;
	}

	i32 BaseBuffer::write(ui32 offset, const void* data, ui32 length) {
		if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
			if (data && length && offset < size) {
				dirty = true;
				length = std::min<ui32>(length, size - offset);
				memcpy((i8*)mapData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	i32 BaseBuffer::update(ui32 offset, const void* data, ui32 length) {
		if ((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);

				glBindBuffer(bufferType, handle);
				glBufferSubData(bufferType, offset, length, data);
				return length;
			}
			return 0;
		}
		return -1;
	}

	void BaseBuffer::flush() {
		if (dirty) {
			waitServerSync();
			sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			dirty = false;
		}
	}

	void BaseBuffer::releaseBuffer() {
		if (handle) {
			releaseSync();

			if (mapData) {
				glBindBuffer(bufferType, handle);
				glUnmapBuffer(bufferType);
				mapData = nullptr;
			}

			glDeleteBuffers(1, &handle);
			handle = 0;

			dirty = false;
		}

		size = 0;
		resUsage = Usage::NONE;
		mapUsage = Usage::NONE;
	}

	void BaseBuffer::waitServerSync() {
		if (sync) {
			do {
				auto rst = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
				if (rst == GL_ALREADY_SIGNALED || rst == GL_CONDITION_SATISFIED) {
					releaseSync();
					return;
				}

			} while (true);
		}
	}

	void BaseBuffer::releaseSync() {
		if (sync) {
			glDeleteSync(sync);
			sync = nullptr;
		}
	}
}