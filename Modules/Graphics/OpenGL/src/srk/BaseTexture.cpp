#include "BaseTexture.h"
#include "Graphics.h"
#include "PixelBuffer.h"
#include "TextureView.h"
#include "srk/Image.h"
#include <algorithm>

namespace srk::modules::graphics::gl {
	BaseTexture::BaseTexture(TextureType texType) :
		dirty(false),
		sampleCount(0),
		texType(texType),
		format(TextureFormat::UNKNOWN),
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

	bool BaseTexture::create(Graphics& graphics, const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount,
		TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data) {
		using namespace srk::enum_operators;

		releaseTex();

		if (sampleCount > 1 && texType != TextureType::TEX2D) {
			graphics.error("OpenGL Texture::create error : only support TextureType::TEX2D when sampleCount > 1");
			return _createDone(graphics, false);
		}

		requiredUsage &= Usage::TEXTURE_RESOURCE_CREATE_ALL;
		preferredUsage &= Usage::TEXTURE_RESOURCE_CREATE_ALL;
		if (auto u = (requiredUsage & (~graphics.getTexCreateUsageMask())); u != Usage::NONE) {
			graphics.error(std::format("OpenGL Texture::create error : has not support Usage {}", (std::underlying_type_t<Usage>)u));
			return _createDone(graphics, false);
		}

		if (!sampleCount) sampleCount = 1;

		if (mipLevels) {
			if (sampleCount > 1) {
				graphics.error("OpenGL Texture::create error : could not enable multisampling and mipmap at same time");
				return _createDone(graphics, false);
			}

			auto maxLevels = Image::calcMipLevels(dim.getMax());
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		} else {
			if (sampleCount > 1) {
				mipLevels = 1;
			} else {
				mipLevels = Image::calcMipLevels(dim.getMax());
			}
		}

		auto isArray = arraySize && texType != TextureType::TEX3D;
		if (arraySize < 1 || texType == TextureType::TEX3D) arraySize = 1;

		auto allUsage = requiredUsage | preferredUsage;
		this->dim.set(dim);

		glGenTextures(1, &handle);

		if (handle) {
			resUsage = requiredUsage | (preferredUsage & graphics.getTexCreateUsageMask());

			if (auto rst = Graphics::convertFormat(format); rst) {
				glTexInfo.internalFormat = rst->internalFormat;
				glTexInfo.format = rst->format;
				glTexInfo.type = rst->type;

				bool supportTexStorage = graphics.getInternalFeatures().supportTexStorage;

				auto perBlockBytes = Image::calcPerBlockBytes(format);
				size_t mipsBytes;
				Image::calcMipsInfo(format, dim, mipLevels, &mipsBytes);

				this->size = mipsBytes * arraySize;

				switch (texType) {
				case TextureType::TEX1D:
				{
					auto w = dim[0];
					if (isArray) {
						glTexInfo.target = GL_TEXTURE_1D_ARRAY;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							glTexStorage2D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, dim[0], arraySize);
							if (data) {
								for (size_t i = 0; i < mipLevels; ++i) {
									for (size_t j = 0; j < arraySize; ++j) {
										if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage2D(glTexInfo.target, i, 0, 0, w, j, glTexInfo.format, glTexInfo.type, subData);
									}
									w = Image::calcNextMipPixels(w);
								}
							}
						} else {
							for (size_t i = 0; i < mipLevels; ++i) {
								for (size_t j = 0; j < arraySize; ++j) glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, w, j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] : nullptr);
								w = Image::calcNextMipPixels(w);
							}
						}
					} else {
						glTexInfo.target = GL_TEXTURE_1D;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							glTexStorage1D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, dim[0]);
							if (data) {
								for (size_t i = 0; i < mipLevels; ++i) {
									if (data[i]) glTexSubImage1D(glTexInfo.target, i, 0, w, glTexInfo.format, glTexInfo.type, data[i]);
									w = Image::calcNextMipPixels(w);
								}
							}
						} else {
							for (size_t i = 0; i < mipLevels; ++i) {
								glTexImage1D(glTexInfo.target, i, glTexInfo.internalFormat, w, 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
								w = Image::calcNextMipPixels(w);
							}
						}
					}

					break;
				}
				case TextureType::TEX2D:
				{
					Vec2uz size2(dim.cast<2>());
					if (isArray) {
						glTexInfo.target = sampleCount > 1 ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							if (sampleCount > 1) {
								glTexStorage3DMultisample(glTexInfo.target, sampleCount, glTexInfo.internalFormat, dim[0], dim[1], arraySize, true);
							} else {
								glTexStorage3D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, dim[0], dim[1], arraySize);
								if (data) {
									for (size_t i = 0; i < mipLevels; ++i) {
										for (size_t j = 0; j < arraySize; ++j) {
											if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size2[0], size2[1], j, glTexInfo.format, glTexInfo.type, subData);
										}
										size2 = Image::calcNextMipPixels(size2);
									}
								}
							}
						} else {
							if (sampleCount > 1) {
								glTexImage3DMultisample(glTexInfo.target, sampleCount, glTexInfo.internalFormat, dim[0], dim[1], arraySize, true);
							} else {
								for (size_t i = 0; i < mipLevels; ++i) {
									for (size_t j = 0; j < arraySize; ++j) glTexImage3D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] : nullptr);
									size2 = Image::calcNextMipPixels(size2);
								}
							}
						}
					} else {
						glTexInfo.target = sampleCount > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							if (sampleCount > 1) {
								glTexStorage2DMultisample(glTexInfo.target, sampleCount, glTexInfo.internalFormat, dim[0], dim[1], true);
							} else {
								glTexStorage2D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, dim[0], dim[1]);
								if (data) {
									for (size_t i = 0; i < mipLevels; ++i) {
										if (data[i]) glTexSubImage2D(glTexInfo.target, i, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, data[i]);
										size2 = Image::calcNextMipPixels(size2);
									}
								}
							}
						} else {
							if (sampleCount > 1) {
								glTexImage2DMultisample(glTexInfo.target, sampleCount, glTexInfo.internalFormat, dim[0], dim[1], true);
							} else {
								for (size_t i = 0; i < mipLevels; ++i) {
									glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
									size2 = Image::calcNextMipPixels(size2);

									/*
									GLuint bufID = 0;
									glGenBuffers(1, &bufID);
									glBindBuffer(GL_PIXEL_UNPACK_BUFFER, bufID);
									glBufferData(GL_PIXEL_UNPACK_BUFFER, this->size, data[0], GL_DYNAMIC_DRAW);
									glBindTexture(glTexInfo.target, handle);
									glTexSubImage2D(glTexInfo.target, i, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, nullptr);
									*/
								}
							}


							//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO
							//glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, size2[1]); // image height in client memory
							//glPixelStorei(GL_UNPACK_ROW_LENGTH, size2[0] * 4); // pitch in client memory

							/*
							const GLbitfield flags =
								GL_MAP_WRITE_BIT |
								GL_MAP_PERSISTENT_BIT |
								GL_MAP_COHERENT_BIT;

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
					auto size3 = dim;
					arraySize = 0;
					glTexInfo.target = GL_TEXTURE_3D;
					glBindTexture(glTexInfo.target, handle);

					if (supportTexStorage) {
						glTexStorage3D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, dim[0], dim[1], dim[2]);
						if (data) {
							for (uint32_t i = 0; i < mipLevels; ++i) {
								if (data[i]) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size3[0], size3[1], size3[2], glTexInfo.format, glTexInfo.type, data[i]);
								size3 = Image::calcNextMipPixels(size3);
							}
						}
					} else {
						for (uint32_t i = 0; i < mipLevels; ++i) {
							glTexImage3D(glTexInfo.target, i, glTexInfo.internalFormat, size3[0], size3[1], size3[2], 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
							size3 = Image::calcNextMipPixels(size3);
						}
					}

					break;
				}
				default:
					break;
				}

				if (sampleCount == 1) glTexParameteri(glTexInfo.target, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
				glBindTexture(glTexInfo.target, 0);

				this->format = format;
				internalArraySize = arraySize;
				this->arraySize = isArray ? arraySize : 0;
				this->mipLevels = mipLevels;
				this->sampleCount = sampleCount;
				//mapData = glMapBufferRange(texType, 0, size, flags);

				//glBindTexture(GL_TEXTURE_BUFFER, handle);
				//mapData = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
				//glBindTexture(GL_TEXTURE_BUFFER, 0);

				return _createDone(graphics, true);
			} else {
				graphics.error("OpenGL Texture::create error : not support texture format");
				return _createDone(graphics, false);
			}
		} else {
			graphics.error("OpenGL Texture::create error : internal texture generate failure");
			return _createDone(graphics, false);
		}
	}

	Usage BaseTexture::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
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

	void BaseTexture::unmap(size_t arraySlice, size_t mipSlice) {
		//mapUsage = Usage::NONE;
	}

	size_t BaseTexture::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		return -1;
	}

	size_t BaseTexture::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
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

	bool BaseTexture::update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data) {
		using namespace srk::enum_operators;

		if (handle && (resUsage & Usage::UPDATE) == Usage::UPDATE && (arraySize ? arraySlice < arraySize : arraySlice == 0) && mipSlice < mipLevels) {
			return _update(arraySlice, mipSlice, range, data);
		}
		return false;
	}

	bool BaseTexture::copyFrom(Graphics& graphics, const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		if (dstArraySlice >= internalArraySize || dstMipSlice >= mipLevels || !src || src->getGraphics() != graphics) return false;

		auto srcBase = (BaseTexture*)src->getNative();
		if (srcArraySlice >= srcBase->internalArraySize || srcMipSlice >= srcBase->mipLevels) return false;

		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		switch (srcBase->texType) {
		case TextureType::TEX1D:
			glFramebufferTexture1D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcBase->glTexInfo.target, srcBase->handle, srcMipSlice);
		case TextureType::TEX2D:
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcBase->glTexInfo.target, srcBase->handle, srcMipSlice);
		case TextureType::TEX3D:
			glFramebufferTexture3D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcBase->glTexInfo.target, srcBase->handle, srcMipSlice, srcRange.pos[2]);
		default:
			break;
		}

		GLenum bindTarget;
		auto isArray = arraySize && texType != TextureType::TEX3D;
		switch (texType) {
		case TextureType::TEX1D:
		{
			if (isArray) {
				bindTarget = GL_TEXTURE_1D_ARRAY;
				glBindTexture(bindTarget, handle);
				glCopyTexSubImage2D(GL_TEXTURE_1D_ARRAY, dstMipSlice, dstPos[0], dstArraySlice, srcRange.pos[0], srcRange.pos[1], srcRange.size[0], srcRange.size[1]);
			} else {
				bindTarget = GL_TEXTURE_1D;
				glBindTexture(bindTarget, handle);
				glCopyTexSubImage1D(GL_TEXTURE_1D, dstMipSlice, dstPos[0], srcRange.pos[0], srcRange.pos[1], srcRange.size[0]);
			}

			break;
		}
		case TextureType::TEX2D:
		{
			if (isArray) {
				bindTarget = GL_TEXTURE_2D_ARRAY;
				glBindTexture(bindTarget, handle);
				glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, dstMipSlice, dstPos[0], dstPos[1], dstArraySlice, srcRange.pos[0], srcRange.pos[1], srcRange.size[0], srcRange.size[1]);
			} else {
				bindTarget = GL_TEXTURE_2D;
				glBindTexture(bindTarget, handle);
				glCopyTexSubImage2D(GL_TEXTURE_1D, dstMipSlice, dstPos[0], dstPos[1], srcRange.pos[0], srcRange.pos[1], srcRange.size[0], srcRange.size[1]);
			}

			break;
		}
		case TextureType::TEX3D:
		{
			bindTarget = GL_TEXTURE_3D;
			glBindTexture(bindTarget, handle);
			glCopyTexSubImage3D(GL_TEXTURE_3D, dstMipSlice, dstPos[0], dstPos[1], dstPos[2], srcRange.pos[0], srcRange.pos[1], srcRange.size[0], srcRange.size[1]);

			break;
		}
		default:
			break;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
		glBindTexture(bindTarget, 0);

		return true;
	}

	bool BaseTexture::copyFrom(Graphics& graphics, size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer) {
		if (!pixelBuffer || graphics != pixelBuffer->getGraphics()) return false;

		auto native = (BaseBuffer*)pixelBuffer->getNative();
		if (!native) return false;

		if (!native->handle || native->bufferType != GL_PIXEL_UNPACK_BUFFER) return false;

		glBindBuffer(native->bufferType, native->handle);
		auto rst = _update(arraySlice, mipSlice, range, nullptr);
		glBindBuffer(native->bufferType, 0);
		native->doSync<true>();

		return rst;
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
			internalArraySize = 0;
			mipLevels = 0;
			dim = 0;
		}

		format = TextureFormat::UNKNOWN;
		sampleCount = 0;
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

	bool BaseTexture::_createDone(Graphics& graphics, bool succeeded) {
		if (!succeeded) releaseTex();
		return succeeded;
	}

	bool BaseTexture::_update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data) {
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
			glBindTexture(glTexInfo.target, 0);
			return false;
		}

		glBindTexture(glTexInfo.target, 0);
		return true;
	}
}