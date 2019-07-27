#include "BaseTexture.h"
#include "Graphics.h"
#include "PixelBuffer.h"
#include "TextureView.h"
#include "base/Image.h"
#include <algorithm>

#include <thread>

namespace aurora::modules::graphics::win_glew {
	BaseTexture::BaseTexture(TextureType texType) :
		dirty(false),
		isArray(false),
		texType(texType),
		resUsage(Usage::NONE),
		mapUsage(Usage::NONE),
		size(0),
		handle(0),
		mapData(nullptr),
		sync(nullptr),
		arraySize(0),
		mipLevels(0) {
		memset(&glTexInfo, 0, sizeof(glTexInfo));
	}

	BaseTexture::~BaseTexture() {
		releaseTex();
	}

	bool BaseTexture::create(Graphics& graphics, const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels,
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
							glTexStorage2D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, size[0], arraySize);
							if (data) {
								for (uint32_t i = 0; i < mipLevels; ++i) {
									for (uint32_t j = 0; j < arraySize; ++j) {
										if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage2D(glTexInfo.target, i, 0, 0, w, j, glTexInfo.format, glTexInfo.type, subData);
									}
									w = Image::calcNextMipPixelSize(w);
								}
							}
						} else {
							for (uint32_t i = 0; i < mipLevels; ++i) {
								for (uint32_t j = 0; j < arraySize; ++j) glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, w, j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] :  nullptr);
								w = Image::calcNextMipPixelSize(w);
							}
						}
					} else {
						glTexInfo.target = GL_TEXTURE_1D;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							glTexStorage1D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, size[0]);
							if (data) {
								for (uint32_t i = 0; i < mipLevels; ++i) {
									if (data[i]) glTexSubImage1D(glTexInfo.target, i, 0, w, glTexInfo.format, glTexInfo.type, data[i]);
									w = Image::calcNextMipPixelSize(w);
								}
							}
						} else {
							for (uint32_t i = 0; i < mipLevels; ++i) {
								glTexImage1D(glTexInfo.target, i, glTexInfo.internalFormat, w, 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
								w = Image::calcNextMipPixelSize(w);
							}
						}
					}

					break;
				}
				case TextureType::TEX2D:
				{
					Vec2ui32 size2(size.slice<2>());
					if (isArray) {
						glTexInfo.target = GL_TEXTURE_2D_ARRAY;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							glTexStorage3D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, size[0], size[1], arraySize);
							if (data) {
								for (uint32_t i = 0; i < mipLevels; ++i) {
									for (uint32_t j = 0; j < arraySize; ++j) {
										if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size2[0], size2[1], j, glTexInfo.format, glTexInfo.type, subData);
									}
									Image::calcNextMipPixelSize(size2);
								}
							}
						} else {
							for (uint32_t i = 0; i < mipLevels; ++i) {
								for (uint32_t j = 0; j < arraySize; ++j) glTexImage3D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] : nullptr);
								Image::calcNextMipPixelSize(size2);
							}
						}
					} else {
						glTexInfo.target = GL_TEXTURE_2D;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							glTexStorage2D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, size[0], size[1]);
							if (data) {
								for (uint32_t i = 0; i < mipLevels; ++i) {
									if (data[i]) glTexSubImage2D(glTexInfo.target, i, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, data[i]);
									Image::calcNextMipPixelSize(size2);
								}
							}
						} else {
							for (uint32_t i = 0; i < mipLevels; ++i) {
								glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
								Image::calcNextMipPixelSize(size2);

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

							/*
							const GLbitfield flags =
								GL_MAP_WRITE_BIT |
								GL_MAP_PERSISTENT_BIT | //在被映射状态下不同步
								GL_MAP_COHERENT_BIT;    //数据对GPU立即可见

							///*
							GLuint bufID = 0;
							glGenBuffers(1, &bufID);
							glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
							//glBufferStorage(GL_PIXEL_UNPACK_BUFFER, this->size, nullptr, flags);
							//auto* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, this->size, flags);
							glBufferData(GL_PIXEL_UNPACK_BUFFER, this->size, nullptr, GL_DYNAMIC_DRAW);

							glTexImage2D(glTexInfo.target, 0, glTexInfo.internalFormat, size2[0], size2[1], 0, glTexInfo.format, glTexInfo.type, nullptr);
							
							//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
							
							auto* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE);
							memcpy(ptr, data[0], this->size);
							//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
							glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

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
						glTexStorage3D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, size[0], size[1], size[2]);
						if (data) {
							for (uint32_t i = 0; i < mipLevels; ++i) {
								if (data[i]) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size3[0], size3[1], size3[2], glTexInfo.format, glTexInfo.type, data[i]);
								Image::calcNextMipPixelSize(size3);
							}
						}
					} else {
						for (uint32_t i = 0; i < mipLevels; ++i) {
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
				//glBindTexture(glTexInfo.target, 0);

				this->isArray = isArray;
				this->arraySize = arraySize;
				this->mipLevels = mipLevels;
				//mapData = glMapBufferRange(texType, 0, size, flags);

				return _createDone(true);
			}
		}

		releaseTex();
		return _createDone(false);
	}

	Usage BaseTexture::map (uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) {
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

	void BaseTexture::unmap (uint32_t arraySlice, uint32_t mipSlice) {
		//mapUsage = Usage::NONE;
	}

	uint32_t BaseTexture::read (uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) {
		return -1;
	}

	uint32_t BaseTexture::write (uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) {
		/*
		if ((mapUsage & Usage::CPU_WRITE) == Usage::CPU_WRITE) {
			if (data && length && offset < size) {
				length = std::min<uint32_t>(length, size - offset);
				memcpy((i8*)mapData + offset, data, length);
				return length;
			}
			return 0;
		}
		*/
		return -1;
	}

	bool BaseTexture::update (uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data) {
		if (handle && (resUsage & Usage::UPDATE) == Usage::UPDATE && arraySlice < arraySize && mipSlice < mipLevels) {
			return _update(arraySlice, mipSlice, range, data);
		}
		return false;
	}

	bool BaseTexture::copyFrom(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) {
		if (pixelBuffer && &graphics == pixelBuffer->getGraphics()) {
			if (auto pb = (PixelBuffer*)pixelBuffer->getNativeBuffer(); pb) {
				if (auto buf = pb->getInternalBuffer(); buf) {
					if (auto pbType = pb->getInternalType(); pbType == GL_PIXEL_UNPACK_BUFFER) {
						glBindBuffer(pbType, buf);
						auto rst = _update(arraySlice, mipSlice, range, nullptr);
						glBindBuffer(pbType, 0);
						pb->baseBuffer.doSync<true>();

						return rst;
					}
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
			memset(&glTexInfo, 0, sizeof(glTexInfo));
			isArray = false;
			arraySize = 0;
			mipLevels = 0;
			texSize.set(0);
		}

		size = 0;
		resUsage = Usage::NONE;
		mapUsage = Usage::NONE;
	}

	void BaseTexture::addView(TextureView& view) {
		if (auto itr = views.find(&view); itr == views.end()) views.emplace(&view);
	}

	void BaseTexture::removeView(TextureView& view) {
		if (auto itr = views.find(&view); itr != views.end()) views.erase(itr);
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

	bool BaseTexture::_createDone(bool succeeded) {
		for (auto& itr : views) itr->onResRecreated();
		return succeeded;
	}

	bool BaseTexture::_update (uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data) {
		glBindTexture(glTexInfo.target, handle);
		switch (glTexInfo.target) {
		case GL_TEXTURE_1D:
			glTexSubImage1D(glTexInfo.target, mipSlice, range.pos[0], range.size[0], glTexInfo.format, glTexInfo.type, data);
			break;
		case GL_TEXTURE_1D_ARRAY:
			glTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], 0, range.size[0], arraySlice, glTexInfo.format, glTexInfo.type, data);
			break;
		case GL_TEXTURE_2D:
			glTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.size[0], range.size[1], glTexInfo.format, glTexInfo.type, data);
			break;
		case GL_TEXTURE_2D_ARRAY:
			glTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], 0, range.size[0], range.size[1], arraySlice, glTexInfo.format, glTexInfo.type, data);
			break;
		case GL_TEXTURE_3D:
			glTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.pos[2], range.size[0], range.size[1], range.size[2], glTexInfo.format, glTexInfo.type, data);
			break;
		default:
		{
			glBindTexture(glTexInfo.target, 0);
			return false;
		}
		}

		glBindTexture(glTexInfo.target, 0);
		return true;
	}
}