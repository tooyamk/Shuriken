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

	bool BaseBuffer::create(Graphics& graphics, uint32_t size, Usage resUsage, const void* data, GLenum internalUsage) {
		releaseBuffer();

		this->resUsage = resUsage & graphics.getCreateBufferMask();
		this->size = size;

		glGenBuffers(1, &handle);
		glBindBuffer(bufferType, handle);

		if ((this->resUsage & Usage::MAP_READ_WRITE) == Usage::NONE) {
			if (data) {
				glBufferData(bufferType, size, data, GL_STATIC_DRAW);
			} else {
				this->resUsage = Usage::UPDATE;
				glBufferData(bufferType, size, nullptr, GL_DYNAMIC_DRAW);
			}
		} else {
			if ((this->resUsage & Usage::PERSISTENT_MAP) == Usage::NONE) {
				glBufferData(bufferType, size, data, internalUsage ? internalUsage : GL_DYNAMIC_DRAW);
			} else {
				GLbitfield flags =
					GL_MAP_PERSISTENT_BIT | //在被映射状态下不同步
					GL_MAP_COHERENT_BIT;    //数据对GPU立即可见

				if ((this->resUsage & Usage::MAP_READ) == Usage::MAP_READ) flags |= GL_MAP_READ_BIT;
				if ((this->resUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) flags |= GL_MAP_WRITE_BIT;

				glBufferStorage(bufferType, size, data, flags);
				mapData = glMapBufferRange(bufferType, 0, size, flags);
			}
		}

		glBindBuffer(bufferType, 0);

		return true;
	}

	Usage BaseBuffer::map(Usage expectMapUsage, GLenum access) {
		Usage ret = Usage::NONE;

		if (handle) {
			auto usage = expectMapUsage & resUsage & Usage::MAP_READ_WRITE;

			if (usage == Usage::NONE) {
				unmap();
			} else {
				ret = usage;

				if (mapUsage != usage) {
					unmap();

					mapUsage = usage;
					if ((this->resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
						_waitServerSync();
					} else {
						if (!access) {
							if (usage == Usage::MAP_READ_WRITE) {
								access = GL_READ_WRITE;
							} else if ((usage & Usage::MAP_READ) != Usage::NONE) {
								access = GL_READ_ONLY;
							} else {
								access = GL_WRITE_ONLY;
							}
						}

						glBindBuffer(bufferType, handle);
						mapData = glMapBuffer(bufferType, access);
						glBindBuffer(bufferType, 0);
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
				doSync<false>();
			} else {
				glBindBuffer(bufferType, handle);
				glUnmapBuffer(bufferType);
				mapData = nullptr;
				glBindBuffer(bufferType, 0);
			}
		}
	}

	uint32_t BaseBuffer::read(uint32_t offset, void* dst, uint32_t dstLen) {
		if ((mapUsage & Usage::MAP_READ)== Usage::MAP_READ) {
			if (dst && dstLen && offset < size) {
				dstLen = std::min<uint32_t>(dstLen, size - offset);
				memcpy(dst, (uint8_t*)mapData + offset, dstLen);
				return dstLen;
			}
			return 0;
		}
		return -1;
	}

	uint32_t BaseBuffer::write(uint32_t offset, const void* data, uint32_t length) {
		if ((mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
			if (data && length && offset < size) {
				dirty = true;
				length = std::min<uint32_t>(length, size - offset);
				memcpy((uint8_t*)mapData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	uint32_t BaseBuffer::update(uint32_t offset, const void* data, uint32_t length) {
		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && length && offset < size) {
				length = std::min<uint32_t>(length, size - offset);

				glBindBuffer(bufferType, handle);
				glBufferSubData(bufferType, offset, length, data);
				glBindBuffer(bufferType, 0);
				return length;
			}
			return 0;
		}
		return -1;
	}

	void BaseBuffer::releaseBuffer() {
		if (handle) {
			releaseSync();

			if (mapData) {
				glBindBuffer(bufferType, handle);
				glUnmapBuffer(bufferType);
				glBindBuffer(bufferType, 0);
				mapData = nullptr;
			}

			glDeleteBuffers(1, &handle);

			dirty = false;
			handle = 0;
		}

		size = 0;
		resUsage = Usage::NONE;
		mapUsage = Usage::NONE;
	}
}