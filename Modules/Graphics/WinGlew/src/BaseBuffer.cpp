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
			this->resUsage = resUsage & (Usage::CPU_READ_WRITE | Usage::GPU_WRITE | (graphics->getDeviceFeatures().supportPersisientMap ? Usage::PERSISTENT_MAP : Usage::NONE));
			this->size = size;

			glBindBuffer(bufferType, handle);

			if ((this->resUsage & Usage::CPU_READ_WRITE) == Usage::NONE) {
				if (data) {
					glBufferData(bufferType, size, data, GL_STATIC_DRAW);
				} else {
					this->resUsage = Usage::GPU_WRITE;
					glBufferData(bufferType, size, nullptr, GL_DYNAMIC_DRAW);
				}
			} else {
				if ((this->resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
					GLbitfield flags = GL_MAP_WRITE_BIT
						| GL_MAP_PERSISTENT_BIT //在被映射状态下不同步
						| GL_MAP_COHERENT_BIT;  //数据对GPU立即可见

					glBufferStorage(bufferType, size, data, flags);
					mapData = glMapBufferRange(bufferType, 0, size, flags);
				} else {
					glBufferData(bufferType, size, data, GL_DYNAMIC_DRAW);
				}
			}

			return true;
		}

		releaseBuffer();
		return false;
	}

	Usage BaseBuffer::map(Usage expectMapUsage) {
		Usage ret = Usage::NONE;

		if (handle) {
			expectMapUsage &= resUsage & Usage::CPU_READ_WRITE;

			if (expectMapUsage == Usage::NONE) {
				unmap();
			} else {
				ret = expectMapUsage;

				if (mapUsage != expectMapUsage) {
					unmap();

					mapUsage = expectMapUsage;
					if ((this->resUsage & Usage::PERSISTENT_MAP) != Usage::PERSISTENT_MAP) {
						GLenum access = 0;
						if (expectMapUsage == Usage::CPU_READ_WRITE) {
							access = GL_READ_WRITE;
						} else if ((expectMapUsage & Usage::CPU_READ) != Usage::NONE) {
							access = GL_READ_ONLY;
						} else {
							access = GL_WRITE_ONLY;
						}

						glBindBuffer(bufferType, handle);
						mapData =  glMapBuffer(bufferType, access);
					}
				}
			}
		}

		return ret;
	}

	void BaseBuffer::unmap() {
		if (mapUsage != Usage::NONE) {
			mapUsage = Usage::NONE;

			if ((this->resUsage & Usage::PERSISTENT_MAP) != Usage::PERSISTENT_MAP) {
				glBindBuffer(bufferType, handle);
				glUnmapBuffer(bufferType);
				mapData = nullptr;
			}
		}
	}

	i32 BaseBuffer::read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return -1;
	}

	i32 BaseBuffer::write(ui32 offset, const void* data, ui32 length) {
		if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
			if (data && length && offset < size) {
				if ((resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) dirty = true;
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