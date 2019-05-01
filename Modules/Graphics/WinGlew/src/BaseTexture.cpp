#include "BaseTexture.h"
#include "Graphics.h"
#include "base/Image.h"
#include <algorithm>

namespace aurora::modules::graphics::win_glew {
	BaseTexture::BaseTexture(TextureType texType) :
		dirty(false),
		texType(texType),
		size(0),
		handle(0),
		mapData(nullptr),
		sync(nullptr),
		internalTexType(0),
		internalFormat(0),
		arraySize(0),
		mipLevels(0),
		resUsage(Usage::NONE),
		mapUsage(Usage::NONE) {
	}

	BaseTexture::~BaseTexture() {
		releaseTex();
	}

	bool BaseTexture::create(Graphics* graphics, const Vec3ui32& size, ui32 arraySize, ui32 mipLevels,
		TextureFormat format, Usage resUsage, const void*const* data) {
		releaseTex();

		if (mipLevels == 0) {
			mipLevels = Image::calcMipLevels(size.getMax());
		} else if (mipLevels > 1) {
			auto maxLevels = Image::calcMipLevels(size.getMax());
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		}

		texSize.set(size);

		glGenTextures(1, &handle);

		if (handle) {
			this->resUsage = resUsage & Usage::GPU_WRITE;
			//this->size = size;

			GLbitfield flags = GL_MAP_WRITE_BIT
				| GL_MAP_PERSISTENT_BIT //在被映射状态下不同步
				| GL_MAP_COHERENT_BIT;  //数据对GPU立即可见

			internalFormat = Graphics::convertInternalFormat(format);

			bool hasTexStorage = graphics->isGreatThanVersion(4, 2);

			switch (texType) {
			case TextureType::TEX1D:
			{
				if (arraySize) {
					internalTexType = GL_TEXTURE_1D_ARRAY;
					glBindTexture(internalTexType, handle);
					if (hasTexStorage) {
						glTexStorage2D(internalTexType, mipLevels, internalFormat, size[0], arraySize);
						if (data) {
							auto w = size[0];
							for (ui32 i = 0; i < mipLevels; ++i) {
								for (ui32 j = 0; j < arraySize; ++j) glTexSubImage2D(internalTexType, i, 0, 0, w, j, internalFormat, GL_UNSIGNED_BYTE, data[i + j * arraySize]);
								w = Image::calcNextMipPixelSize(w);
							}
						}
					} else {
						auto w = size[0];
						for (ui32 i = 0; i < mipLevels; ++i) {
							for (ui32 j = 0; j < arraySize; ++j) glTexImage2D(internalTexType, i, internalFormat, w, j, 0, internalFormat, GL_UNSIGNED_BYTE, data[i + j * arraySize]);
							w = Image::calcNextMipPixelSize(w);
						}
					}
				} else {
					internalTexType = GL_TEXTURE_1D;
					glBindTexture(internalTexType, handle);
					if (hasTexStorage) {
						glTexStorage1D(internalTexType, mipLevels, internalFormat, size[0]);
						if (data) {
							auto w = size[0];
							for (ui32 i = 0; i < mipLevels; ++i) {
								glTexSubImage1D(internalTexType, i, 0, w, internalFormat, GL_UNSIGNED_BYTE, data[i]);
								w = Image::calcNextMipPixelSize(w);
							}
						}
					} else {
						auto w = size[0];
						for (ui32 i = 0; i < mipLevels; ++i) {
							glTexImage1D(internalTexType, i, internalFormat, w, 0, internalFormat, GL_UNSIGNED_BYTE, data[i]);
							w = Image::calcNextMipPixelSize(w);
						}
					}
				}

				break;
			}
			case TextureType::TEX2D:
			{
				if (arraySize) {
					internalTexType = GL_TEXTURE_2D_ARRAY;
					glBindTexture(internalTexType, handle);
					if (hasTexStorage) {
						glTexStorage3D(internalTexType, mipLevels, internalFormat, size[0], size[1], arraySize);
						if (data) {
							Vec2ui32 size2((ui32(&)[2])size);
							for (ui32 i = 0; i < mipLevels; ++i) {
								for (ui32 j = 0; j < arraySize; ++j) glTexSubImage3D(internalTexType, i, 0, 0, 0, size2[0], size2[1], j, internalFormat, GL_UNSIGNED_BYTE, data[i + j * arraySize]);
								Image::calcNextMipPixelSize(size2);
							}
						}
					} else {
						Vec2ui32 size2((ui32(&)[2])size);
						for (ui32 i = 0; i < mipLevels; ++i) {
							for (ui32 j = 0; j < arraySize; ++j) glTexImage3D(internalTexType, i, internalFormat, size2[0], size2[1], j, 0, internalFormat, GL_UNSIGNED_BYTE, data[i + j * arraySize]);
							Image::calcNextMipPixelSize(size2);
						}
					}
				} else {
					internalTexType = GL_TEXTURE_2D;
					glBindTexture(internalTexType, handle);
					if (hasTexStorage) {
						glTexStorage2D(internalTexType, mipLevels, internalFormat, size[0], size[1]);
						if (data) {
							Vec2ui32 size2((ui32(&)[2])size);
							for (ui32 i = 0; i < mipLevels; ++i) {
								for (ui32 j = 0; j < arraySize; ++j) glTexSubImage2D(internalTexType, i, 0, 0, size2[0], size2[1], internalFormat, GL_UNSIGNED_BYTE, data[i]);
								Image::calcNextMipPixelSize(size2);
							}
						}
					} else {
						Vec2ui32 size2((ui32(&)[2])size);
						for (ui32 i = 0; i < mipLevels; ++i) {
							glTexImage2D(internalTexType, i, internalFormat, size2[0], size2[1], 0, internalFormat, GL_UNSIGNED_BYTE, data[i]);
							Image::calcNextMipPixelSize(size2);
						}
					}
				}

				break;
			}
			case TextureType::TEX3D:
			{
				arraySize = 0;
				internalTexType = GL_TEXTURE_3D;
				glBindTexture(internalTexType, handle);
				if (hasTexStorage) {
					glTexStorage3D(internalTexType, mipLevels, internalFormat, size[0], size[1], size[2]);
					if (data) {
						auto size3 = size;
						for (ui32 i = 0; i < mipLevels; ++i) {
							glTexSubImage3D(internalTexType, i, 0, 0, 0, size3[0], size3[1], size3[2], internalFormat, GL_UNSIGNED_BYTE, data[i]);
							Image::calcNextMipPixelSize(size3);
						}
					}
				} else {
					auto size3 = size;
					for (ui32 i = 0; i < mipLevels; ++i) {
						glTexImage3D(internalTexType, i, internalFormat, size3[0], size3[1], size3[2], 0, internalFormat, GL_UNSIGNED_BYTE, data[i]);
						Image::calcNextMipPixelSize(size3);
					}
				}

				break;
			}
			default:
				break;
			}

			this->arraySize = arraySize;
			this->mipLevels = mipLevels;
			//mapData = glMapBufferRange(texType, 0, size, flags);

			return true;
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

	i32 BaseTexture::read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		return -1;
	}

	i32 BaseTexture::write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) {
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
		if ((resUsage & Usage::GPU_WRITE) == Usage::GPU_WRITE && internalTexType && arraySlice < arraySize && mipSlice < mipLevels) {
			switch (internalTexType) {
			case GL_TEXTURE_1D:
			{
				glBindTexture(internalTexType, handle);
				glTexSubImage1D(internalTexType, mipSlice, range.pos[0], range.size[0], internalFormat, GL_UNSIGNED_BYTE, data);

				return true;
			}
			case GL_TEXTURE_1D_ARRAY:
			{
				glBindTexture(internalTexType, handle);
				glTexSubImage2D(internalTexType, mipSlice, range.pos[0], 0, range.size[0], arraySlice, internalFormat, GL_UNSIGNED_BYTE, data);

				return true;
			}
			case GL_TEXTURE_2D:
			{
				glBindTexture(internalTexType, handle);
				glTexSubImage2D(internalTexType, mipSlice, range.pos[0], range.pos[1], range.size[0], range.size[1], internalFormat, GL_UNSIGNED_BYTE, data);

				return true;
			}
			case GL_TEXTURE_2D_ARRAY:
			{
				glBindTexture(internalTexType, handle);
				glTexSubImage3D(internalTexType, mipSlice, range.pos[0], range.pos[1], 0, range.size[0], range.size[1], arraySlice, internalFormat, GL_UNSIGNED_BYTE, data);

				return true;
			}
			case GL_TEXTURE_3D:
			{
				glBindTexture(internalTexType, handle);
				glTexSubImage3D(internalTexType, mipSlice, range.pos[0], range.pos[1], range.pos[2], range.size[0], range.size[1], range.size[2], internalFormat, GL_UNSIGNED_BYTE, data);

				return true;
			}
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
			internalTexType = 0;
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