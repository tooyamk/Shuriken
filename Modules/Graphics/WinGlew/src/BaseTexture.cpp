#include "BaseTexture.h"
#include "Graphics.h"
#include "base/Image.h"
#include <algorithm>

#include <thread>

namespace aurora::modules::graphics::win_glew {
	BaseTexture::BaseTexture(TextureType texType) :
		dirty(false),
		texType(texType),
		size(0),
		handle(0),
		mapData(nullptr),
		sync(nullptr),
		arraySize(0),
		mipLevels(0),
		resUsage(Usage::NONE),
		mapUsage(Usage::NONE) {
		memset(&glTexInfo, 0, sizeof(glTexInfo));
	}

	BaseTexture::~BaseTexture() {
		releaseTex();
	}

	bool BaseTexture::create(Graphics& graphics, const Vec3ui32& size, ui32 arraySize, ui32 mipLevels,
		TextureFormat format, Usage resUsage, const void*const* data) {
		releaseTex();

		if (mipLevels == 0) {
			mipLevels = Image::calcMipLevels(size.getMax());
		} else if (mipLevels > 1) {
			auto maxLevels = Image::calcMipLevels(size.getMax());
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		}

		auto isArray = arraySize && texType != TextureType::TEX3D;
		if (arraySize < 1 || texType == TextureType::TEX3D) arraySize = 1;

		texSize.set(size);

		glGenTextures(1, &handle);

		if (handle) {
			this->resUsage = resUsage & graphics.getCreateBufferMask();

			GLbitfield flags = GL_MAP_WRITE_BIT
				| GL_MAP_PERSISTENT_BIT //在被映射状态下不同步
				| GL_MAP_COHERENT_BIT;  //数据对GPU立即可见

			Graphics::convertFormat(format, glTexInfo.internalFormat, glTexInfo.format, glTexInfo.type);

			if (glTexInfo.format != 0) {
				bool supportTexStorage = graphics.getInternalFeatures().supportTexStorage;

				auto perPixelSize = Image::calcPerPixelByteSize(format);
				auto mipsByteSize = Image::calcMipsByteSize(size, mipLevels, perPixelSize);

				this->size = mipsByteSize * arraySize;

				switch (texType) {
				case TextureType::TEX1D:
				{
					auto w = size[0];
					if (isArray) {
						glTexInfo.target = GL_TEXTURE_1D_ARRAY;
						glBindTexture(glTexInfo.target, handle);
						if (supportTexStorage) {
							for (ui32 i = 0; i < mipLevels; ++i) {
								for (ui32 j = 0; j < arraySize; ++j) {
									glTexStorage2D(glTexInfo.target, i, glTexInfo.internalFormat, size[0], j);
									if (data) {
										if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage2D(glTexInfo.target, i, 0, 0, w, j, glTexInfo.format, glTexInfo.type, subData);
									}

								}
								w = Image::calcNextMipPixelSize(w);
							}
						} else {
							for (ui32 i = 0; i < mipLevels; ++i) {
								for (ui32 j = 0; j < arraySize; ++j) glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, w, j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] :  nullptr);
								w = Image::calcNextMipPixelSize(w);
							}
						}
					} else {
						glTexInfo.target = GL_TEXTURE_1D;
						glBindTexture(glTexInfo.target, handle);
						if (supportTexStorage) {
							for (ui32 i = 0; i < mipLevels; ++i) {
								glTexStorage1D(glTexInfo.target, i, glTexInfo.internalFormat, size[0]);
								if (data && data[i]) glTexSubImage1D(glTexInfo.target, i, 0, w, glTexInfo.format, glTexInfo.type, data[i]);
								w = Image::calcNextMipPixelSize(w);
							}
						} else {
							for (ui32 i = 0; i < mipLevels; ++i) {
								glTexImage1D(glTexInfo.target, i, glTexInfo.internalFormat, w, 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
								w = Image::calcNextMipPixelSize(w);
							}
						}
					}

					break;
				}
				case TextureType::TEX2D:
				{
					Vec2ui32 size2((ui32(&)[2])size);
					if (isArray) {
						glTexInfo.target = GL_TEXTURE_2D_ARRAY;
						glBindTexture(glTexInfo.target, handle);
						if (supportTexStorage) {
							for (ui32 i = 0; i < mipLevels; ++i) {
								for (ui32 j = 0; j < arraySize; ++j) {
									glTexStorage3D(glTexInfo.target, i, glTexInfo.internalFormat, size[0], size[1], j);
									if (data) {
										if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size2[0], size2[1], j, glTexInfo.format, glTexInfo.type, subData);
									}
								}
								Image::calcNextMipPixelSize(size2);
							}
						} else {
							for (ui32 i = 0; i < mipLevels; ++i) {
								for (ui32 j = 0; j < arraySize; ++j) glTexImage3D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] : nullptr);
								Image::calcNextMipPixelSize(size2);
							}
						}
					} else {
						glTexInfo.target = GL_TEXTURE_2D;
						glBindTexture(glTexInfo.target, handle);
						if (supportTexStorage) {
							for (ui32 i = 0; i < mipLevels; ++i) {
								glTexStorage2D(glTexInfo.target, i, glTexInfo.internalFormat, size[0], size[1]);
								if (data && data[i]) glTexSubImage2D(glTexInfo.target, i, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, data[i]);
								Image::calcNextMipPixelSize(size2);
							}
						} else {
							for (ui32 i = 0; i < mipLevels; ++i) {
								//glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
								//Image::calcNextMipPixelSize(size2);

								/*
								GLuint bufID = 0;
								glGenBuffers(1, &bufID);
								glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
								glBufferData(GL_PIXEL_UNPACK_BUFFER, this->size, data[0], GL_DYNAMIC_DRAW);
								glBindTexture(glTexInfo.target, handle);
								glTexSubImage2D(glTexInfo.target, i, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, nullptr);
								*/
							}


							//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO
							//glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, size2[1]); // image height in client memory
							//glPixelStorei(GL_UNPACK_ROW_LENGTH, size2[0] * 4); // pitch in client memory

							const GLbitfield flags =
								GL_MAP_WRITE_BIT |
								GL_MAP_PERSISTENT_BIT | //在被映射状态下不同步
								GL_MAP_COHERENT_BIT;    //数据对GPU立即可见

							///*
							GLuint bufID = 0;
							glGenBuffers(1, &bufID);
							glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
							glBufferStorage(GL_PIXEL_UNPACK_BUFFER, this->size, nullptr, flags);
							auto* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, this->size, flags);
							//glBufferData(GL_PIXEL_UNPACK_BUFFER, this->size, nullptr, GL_STREAM_READ);

							glTexImage2D(glTexInfo.target, 0, glTexInfo.internalFormat, size2[0], size2[1], 0, glTexInfo.format, glTexInfo.type, nullptr);
							
							//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
							
							//auto* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE);
							memcpy(ptr, data[0], this->size);
							//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
							//glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

							glTexSubImage2D(glTexInfo.target, 0, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, nullptr);
							//glActiveTexture(GL_TEXTURE0);
							//glBindTexture(glTexInfo.target, handle);
							//glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // TODO
							//glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, size2[1]); // image height in client memory
							//glPixelStorei(GL_UNPACK_ROW_LENGTH, size2[0] * 4); // pitch in client memory
							//glTexImage2D(glTexInfo.target, 0, glTexInfo.internalFormat, size2[0], size2[1], 0, glTexInfo.format, glTexInfo.type, nullptr);
							//glTexSubImage2D(glTexInfo.target, 0, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, nullptr);
							//glBindTexture(glTexInfo.target, 0);
							glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
							//*/
						}
					}

					break;
				}
				case TextureType::TEX3D:
				{
					auto size3 = size;
					arraySize = 0;
					glTexInfo.target = GL_TEXTURE_3D;
					glBindTexture(glTexInfo.target, handle);
					if (supportTexStorage) {
						for (ui32 i = 0; i < mipLevels; ++i) {
							glTexStorage3D(glTexInfo.target, i, glTexInfo.internalFormat, size[0], size[1], size[2]);
							if (data && data[i]) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size3[0], size3[1], size3[2], glTexInfo.format, glTexInfo.type, data[i]);
							Image::calcNextMipPixelSize(size3);
						}
					} else {
						for (ui32 i = 0; i < mipLevels; ++i) {
							glTexImage3D(glTexInfo.target, i, glTexInfo.internalFormat, size3[0], size3[1], size3[2], 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
							Image::calcNextMipPixelSize(size3);
						}
					}

					break;
				}
				default:
					break;
				}

				glTexParameteri(glTexInfo.target, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
				glBindTexture(glTexInfo.target, 0);

				this->arraySize = arraySize ? arraySize : 1;
				this->mipLevels = mipLevels;
				//mapData = glMapBufferRange(texType, 0, size, flags);

				return true;
			}
		}

		releaseTex();
		return false;
	}

	Usage BaseTexture::map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) {
		return Usage::NONE;
		/*
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
					ret |= Usage::CPU_WRITE | Usage::CPU_WRITE_NO_OVERWRITE;
				}
			} else {
				expectMapUsage &= ~Usage::CPU_WRITE;
			}

			mapUsage = expectMapUsage;
		}

		return ret;
		*/
	}

	void BaseTexture::unmap(ui32 arraySlice, ui32 mipSlice) {
		//mapUsage = Usage::NONE;
	}

	ui32 BaseTexture::read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen) {
		return -1;
	}

	ui32 BaseTexture::write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) {
		/*
		if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<ui32>(length, size - offset);
				memcpy((i8*)mapData + offset, data, length);
				return length;
			}
			return 0;
		}
		*/
		return -1;
	}

	bool BaseTexture::update(ui32 arraySlice, ui32 mipSlice, const Box3ui32& range, const void* data) {
		if (handle && (resUsage & Usage::UPDATE) == Usage::UPDATE && arraySlice < arraySize && mipSlice < mipLevels) {
			glBindTexture(glTexInfo.internalFormat, handle);
			switch (glTexInfo.target) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(glTexInfo.target, mipSlice, range.pos[0], range.size[0], glTexInfo.format, glTexInfo.type, data);
				return true;
			case GL_TEXTURE_1D_ARRAY:
				glTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], 0, range.size[0], arraySlice, glTexInfo.format, glTexInfo.type, data);
				return true;
			case GL_TEXTURE_2D:
				glTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.size[0], range.size[1], glTexInfo.format, glTexInfo.type, data);
				return true;
			case GL_TEXTURE_2D_ARRAY:
				glTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], 0, range.size[0], range.size[1], arraySlice, glTexInfo.format, glTexInfo.type, data);
				return true;
			case GL_TEXTURE_3D:
				glTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.pos[2], range.size[0], range.size[1], range.size[2], glTexInfo.format, glTexInfo.type, data);
				return true;
			}
		}
		return false;
	}

	void BaseTexture::flush() {
		if (dirty) {
			waitServerSync();
			sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			dirty = false;
		}
	}

	void BaseTexture::releaseTex() {
		if (handle) {
			releaseSync();

			if (mapData) {
				//glBindBuffer(texType, handle);
				//glUnmapBuffer(texType);
				mapData = nullptr;
			}

			glDeleteTextures(1, &handle);
			handle = 0;

			dirty = false;
			memset(&glTexInfo, 0, sizeof(glTexInfo));
			arraySize = 0;
			mipLevels = 0;
			texSize.set(0);
		}

		size = 0;
		resUsage = Usage::NONE;
		mapUsage = Usage::NONE;
	}

	void BaseTexture::waitServerSync() {
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

	void BaseTexture::releaseSync() {
		if (sync) {
			glDeleteSync(sync);
			sync = nullptr;
		}
	}
}