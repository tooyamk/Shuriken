#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_glew {
	BaseBuffer::BaseBuffer(GLenum bufferType) :
		dirty(false),
		numBuffers(0),
		resUsage(Usage::NONE),
		mapUsage(Usage::NONE),
		bufferType(bufferType),
		size(0),
		curHandle(0) {
		memset(&bufferData, 0, sizeof(bufferData));
	}

	BaseBuffer::~BaseBuffer() {
		releaseBuffer();
	}

	bool BaseBuffer::create(Graphics& graphics, ui32 size, Usage resUsage, const void* data) {
		releaseBuffer();

		this->resUsage = resUsage & graphics.getCreateBufferMask();
		this->size = size;

		if ((this->resUsage & Usage::MAP_READ_WRITE) == Usage::NONE) {
			numBuffers = 1;

			glGenBuffers(1, &bufferData.handle);
			curHandle = bufferData.handle;
			glBindBuffer(bufferType, curHandle);

			if (data) {
				glBufferData(bufferType, size, data, GL_STATIC_DRAW);
			} else {
				this->resUsage = Usage::UPDATE;
				glBufferData(bufferType, size, nullptr, GL_DYNAMIC_DRAW);
			}
		} else {
			if ((this->resUsage & Usage::PERSISTENT_MAP) == Usage::NONE) {
				numBuffers = 1;

				glGenBuffers(1, &bufferData.handle);
				curHandle = bufferData.handle;
				glBindBuffer(bufferType, curHandle);

				glBufferData(bufferType, size, data, GL_DYNAMIC_DRAW);
			} else {
				numBuffers = (((ui32)this->resUsage >> (Math::potLog2((ui32)Usage::PERSISTENT_MAP) + 1)) & 0b11) + 1;

				GLuint* handles = nullptr;
				void** mapDatas = nullptr;
				GLsync* syncs = nullptr;
				if (numBuffers == 1) {
					handles = &bufferData.handle;
					mapDatas = &bufferData.mapData;
					syncs = &bufferData.sync;
				} else {
					handles = new GLuint[numBuffers];
					mapDatas = new void*[numBuffers];
					syncs = new GLsync[numBuffers];
					bufferData.handles = handles;
					bufferData.mapDatas = mapDatas;
					bufferData.syncs = syncs;
				}

				glGenBuffers(numBuffers, handles);
				curHandle = handles[0];

				_createPersistentMapBuffer(handles[0], mapDatas[0], data);
				syncs[0] = nullptr;

				for (ui32 i = 1; i < numBuffers; ++i) {
					bufferData.mapDatas[i] = nullptr;
					bufferData.syncs[i] = nullptr;
				}
			}
		}

		return true;
	}

	Usage BaseBuffer::map(Usage expectMapUsage) {
		Usage ret = Usage::NONE;

		if (numBuffers) {
			auto usage = expectMapUsage & resUsage & Usage::MAP_READ_WRITE;

			if (usage == Usage::NONE) {
				unmap();
			} else {
				ret = usage;

				if (mapUsage != usage) {
					unmap();

					mapUsage = usage;
					if ((this->resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
						if (numBuffers > 1 && (expectMapUsage & Usage::MAP_SWAP) == Usage::MAP_SWAP) {
							if (auto& sync = _getCurSync(); sync && _isSyncing(sync)) {
								if (++bufferData.curIndex >= numBuffers) bufferData.curIndex = 0;
								curHandle = bufferData.handles[bufferData.curIndex];
								if (!bufferData.mapDatas[bufferData.curIndex]) _createPersistentMapBuffer(bufferData.handles[bufferData.curIndex], bufferData.mapDatas[bufferData.curIndex], nullptr);

								ret |= Usage::DISCARD;
							}
						}

						_waitServerSync();
					} else {
						GLenum access = 0;
						if (usage == Usage::MAP_READ_WRITE) {
							access = GL_READ_WRITE;
						} else if ((usage & Usage::MAP_READ) != Usage::NONE) {
							access = GL_READ_ONLY;
						} else {
							access = GL_WRITE_ONLY;
						}

						glBindBuffer(bufferType, curHandle);
						bufferData.mapData = glMapBuffer(bufferType, access);
					}
				}
			}
		}

		return ret;
	}

	void BaseBuffer::unmap() {
		if (mapUsage != Usage::NONE) {
			mapUsage = Usage::NONE;

			if ((this->resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
				flush();
			} else {
				glBindBuffer(bufferType, curHandle);
				glUnmapBuffer(bufferType);
				bufferData.mapData = nullptr;
			}
		}
	}

	ui32 BaseBuffer::read(ui32 offset, void* dst, ui32 dstLen) {
		if ((mapUsage & Usage::MAP_READ)== Usage::MAP_READ) {
			if (dst && dstLen && offset < size) {
				dstLen = std::min<ui32>(dstLen, size - offset);
				memcpy(dst, (i8*)_getCurMapData() + offset, dstLen);
				return dstLen;
			}
			return 0;
		}
		return -1;
	}

	ui32 BaseBuffer::write(ui32 offset, const void* data, ui32 length) {
		if ((mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
			if (data && length && offset < size) {
				if ((resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) dirty = true;
				length = std::min<ui32>(length, size - offset);
				memcpy((i8*)_getCurMapData() + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	ui32 BaseBuffer::update(ui32 offset, const void* data, ui32 length) {
		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);

				glBindBuffer(bufferType, curHandle);
				glBufferSubData(bufferType, offset, length, data);
				return length;
			}
			return 0;
		}
		return -1;
	}

	void BaseBuffer::flush() {
		if (dirty) {
			_waitServerSync();
			_getCurSync() = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			dirty = false;
		}
	}

	void BaseBuffer::releaseBuffer() {
		if (numBuffers) {
			if (numBuffers == 1) {
				_releaseSync(bufferData.sync);

				if (bufferData.mapData) {
					glBindBuffer(bufferType, curHandle);
					glUnmapBuffer(bufferType);
				}

				glDeleteBuffers(1, &curHandle);
			} else {
				for (ui32 i = 0; i < numBuffers; ++i) {
					_releaseSync(bufferData.syncs[i]);

					if (bufferData.mapDatas[i]) {
						glBindBuffer(bufferType, bufferData.handles[i]);
						glUnmapBuffer(bufferType);
					}
				}

				glDeleteBuffers(numBuffers, bufferData.handles);

				delete[] bufferData.handles;
				delete[] bufferData.mapDatas;
				delete[] bufferData.syncs;
			}

			dirty = false;
			numBuffers = 0;
			curHandle = 0;
		}

		size = 0;
		resUsage = Usage::NONE;
		mapUsage = Usage::NONE;
		memset(&bufferData, 0, sizeof(bufferData));
	}

	void BaseBuffer::_createPersistentMapBuffer(GLuint handle, void*& mapData, const void* data) {
		const GLbitfield flags =
			GL_MAP_WRITE_BIT |
			GL_MAP_PERSISTENT_BIT | //在被映射状态下不同步
			GL_MAP_COHERENT_BIT;    //数据对GPU立即可见

		glBindBuffer(bufferType, handle);
		glBufferStorage(bufferType, size, data, flags);
		mapData = glMapBufferRange(bufferType, 0, size, flags);
	}
}