#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>
#include <format>

namespace srk::modules::graphics::gl {
	BaseBuffer::BaseBuffer(GLenum bufferType) :
		mapDirty(false),
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

	bool BaseBuffer::create(Graphics& graphics, size_t size, Usage requiredUsage, Usage preferredUsage, const void* data, GLenum internalUsage) {
		using namespace srk::enum_operators;

		releaseBuffer();

		requiredUsage &= Usage::BUFFER_CREATE_ALL;
		preferredUsage &= Usage::BUFFER_CREATE_ALL;
		if (auto u = (requiredUsage & (~graphics.getBufferCreateUsageMask())); u != Usage::NONE) {
			graphics.error(std::format("OpenGL BaseBuffer::create error : has not support Usage {}", (std::underlying_type_t<Usage>)u));
			return false;
		}

		auto allUsage = requiredUsage | preferredUsage;

		auto persistentMap = (allUsage & Usage::PERSISTENT_MAP) != Usage::NONE;
		if (persistentMap && !graphics.getDeviceFeatures().persistentMap) {
			if ((requiredUsage & Usage::PERSISTENT_MAP) == Usage::NONE) {
				persistentMap = false;
				allUsage &= ~Usage::PERSISTENT_MAP;
			} else {
				graphics.error("OpenGL BaseBuffer::create error : not support Usage::PERSISTENT_MAP");
				return false;
			}
		}

		resUsage = Usage::NONE;
		this->size = size;

		glGenBuffers(1, &handle);
		glBindBuffer(bufferType, handle);

		auto cpuRead = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
		auto cpuWrite = (allUsage & Usage::MAP_WRITE_UPDATE) != Usage::NONE;

		if (persistentMap) {
			resUsage |= Usage::PERSISTENT_MAP;
			GLbitfield flags = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

			if (cpuRead) {
				resUsage |= Usage::MAP_READ;
				flags |= GL_MAP_READ_BIT;
			}
			if (cpuWrite) {
				resUsage |= Usage::MAP_WRITE;
				flags |= GL_MAP_WRITE_BIT;
			}

			glBufferStorage(bufferType, size, data, flags);
			mapData = glMapBufferRange(bufferType, 0, size, flags);
		} else {
			if (cpuRead) resUsage |= Usage::MAP_READ;
			if (cpuWrite) resUsage |= Usage::MAP_WRITE;

			GLint glUsage;
			if (internalUsage) {
				glUsage = internalUsage;
			} else {
				if (cpuWrite || ((allUsage & Usage::COPY_DST) == Usage::COPY_DST)) {
					glUsage = cpuRead ? GL_DYNAMIC_READ : GL_DYNAMIC_DRAW;
				} else {
					glUsage = cpuRead ? GL_STATIC_READ : GL_STATIC_DRAW;
				}
			}
			glBufferData(bufferType, size, data, glUsage);
		}

		glBindBuffer(bufferType, 0);

		resUsage |= allUsage & (Usage::UPDATE | Usage::COPY_SRC_DST);
		if ((resUsage & requiredUsage) != requiredUsage) {
			graphics.error(std::format("OpenGL BaseBuffer::create error : has not support preferredUsage {}", (std::underlying_type_t<Usage>)(requiredUsage & (~(resUsage & requiredUsage)))));
			releaseBuffer();
			return false;
		}

		return true;
	}

	Usage BaseBuffer::map(Usage expectMapUsage, GLenum access) {
		using namespace srk::enum_operators;

		auto ret = Usage::NONE;

		if (handle) {
			auto usage = expectMapUsage & resUsage & Usage::MAP_READ_WRITE;

			if (usage == Usage::NONE) {
				unmap();
			} else {
				ret = usage;

				if (mapUsage != usage) {
					unmap();

					mapUsage = usage;
					if ((resUsage & Usage::PERSISTENT_MAP) == Usage::PERSISTENT_MAP) {
						_waitServerSync();
					} else {
						if (!access) {
							if ((resUsage & Usage::MAP_READ) == Usage::MAP_READ) {
								access = (resUsage & Usage::MAP_WRITE) != Usage::NONE ? GL_READ_WRITE : GL_READ_ONLY;
							} else {
								access = GL_WRITE_ONLY;
							}

							/*if (usage == Usage::MAP_READ_WRITE) {
								access = GL_READ_WRITE;
							} else if ((usage & Usage::MAP_READ) != Usage::NONE) {
								access = GL_READ_ONLY;
							} else {
								access = GL_WRITE_ONLY;
							}*/
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
		using namespace srk::enum_operators;

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

	size_t BaseBuffer::read(void* dst, size_t dstLen, size_t offset) {
		using namespace srk::enum_operators;

		if ((mapUsage & Usage::MAP_READ)== Usage::MAP_READ) {
			if (dst && dstLen && offset < size) {
				dstLen = std::min<size_t>(dstLen, size - offset);
				memcpy(dst, (uint8_t*)mapData + offset, dstLen);
				return dstLen;
			}
			return 0;
		}
		return -1;
	}

	size_t BaseBuffer::write(const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
			if (data && length && offset < size) {
				mapDirty = true;
				length = std::min<size_t>(length, size - offset);
				memcpy((uint8_t*)mapData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	size_t BaseBuffer::update(const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((resUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && length && offset < size) {
				length = std::min<size_t>(length, size - offset);

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

			mapDirty = false;
			handle = 0;
		}

		size = 0;
		resUsage = Usage::NONE;
		mapUsage = Usage::NONE;
	}

	size_t BaseBuffer::copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange) {
		if (dstPos >= size || srcRange.pos[0] >= src.size) return 0;

		auto copySize = std::min(std::min(src.size - srcRange.pos[0], srcRange.size[0]), size - dstPos);

		glBindBuffer(GL_COPY_READ_BUFFER, src.handle);
		glBindBuffer(GL_COPY_WRITE_BUFFER, handle);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcRange.pos[0], dstPos, copySize);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

		return copySize;
	}

	size_t BaseBuffer::copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		using namespace srk::enum_operators;

		if (!handle || (resUsage & Usage::COPY_DST) != Usage::COPY_DST || !src || (src->getUsage() & Usage::COPY_SRC) != Usage::COPY_SRC || src->getGraphics() != graphics) return -1;

		auto srcNative = (const BaseBuffer*)src->getNative();
		if (!srcNative || !srcNative->handle) return -1;

		return copyFrom(graphics, dstPos, *srcNative, srcRange);
	}
}