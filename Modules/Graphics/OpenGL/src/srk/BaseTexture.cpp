#include "BaseTexture.h"
#include "Graphics.h"
#include "TextureView.h"
#include "srk/ScopePtr.h"
#include "srk/String.h"
#include <algorithm>

namespace srk::modules::graphics::gl {
	BaseTexture::PixelBuffer::PixelBuffer() :
		_baseBuffer(0) {
	}

	bool BaseTexture::PixelBuffer::create(Graphics& graphics, size_t size, Usage requiredUsage, Usage preferredUsage, const void* data, size_t dataSize) {
		using namespace srk::enum_operators;

		auto allUsage = requiredUsage | preferredUsage;
		auto read = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
		auto write = (allUsage & Usage::MAP_WRITE) != Usage::NONE;

		if (read && write) {
			auto requiredRead = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
			auto requiredWrite = (allUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE;
			if (requiredRead) {
				if (requiredWrite) {
					return false;
				} else {
					write = false;
					preferredUsage &= ~Usage::MAP_WRITE;
				}
			} else if (requiredWrite) {
				read = false;
				preferredUsage &= ~Usage::MAP_READ;
			} else {
				write = false;
				preferredUsage &= ~Usage::MAP_WRITE;
			}
		}

		if (read) {
			_baseBuffer.bufferType = GL_PIXEL_PACK_BUFFER;
		} else if (write) {
			_baseBuffer.bufferType = GL_PIXEL_UNPACK_BUFFER;
		} else {
			return true;
		}

		return _baseBuffer.create(graphics, size, requiredUsage, preferredUsage, data, GL_DYNAMIC_READ);
	}

	Usage BaseTexture::PixelBuffer::map(Usage expectMapUsage) {
		return _baseBuffer.map(expectMapUsage, GL_READ_WRITE);
	}

	void BaseTexture::PixelBuffer::unmap() {
		_baseBuffer.unmap();
	}

	size_t BaseTexture::PixelBuffer::read(void* dst, size_t dstLen, size_t offset) {
		return _baseBuffer.read(dst, dstLen, offset);
	}

	size_t BaseTexture::PixelBuffer::write(const void* data, size_t length, size_t offset) {
		return _baseBuffer.write(data, length, offset);
	}

	bool BaseTexture::PixelBuffer::copyFrom(uint32_t mipSlice, const BaseTexture* src) {
		if (!_baseBuffer.handle || _baseBuffer.bufferType != GL_PIXEL_PACK_BUFFER) return false;

		glBindBuffer(_baseBuffer.bufferType, _baseBuffer.handle);
		glBindTexture(src->glTexInfo.target, src->handle);

		glGetTexImage(src->glTexInfo.target, mipSlice, src->glTexInfo.format, src->glTexInfo.type, nullptr);

		glBindTexture(src->glTexInfo.target, 0);
		glBindBuffer(_baseBuffer.bufferType, 0);

		return true;
	}

	void BaseTexture::PixelBuffer::copyFrom(Graphics& graphics, const PixelBuffer* src) {
		glBindBuffer(GL_COPY_READ_BUFFER, src->_baseBuffer.handle);
		glBindBuffer(GL_COPY_WRITE_BUFFER, _baseBuffer.handle);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _baseBuffer.size);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	}


	BaseTexture::BaseTexture(TextureType texType) :
		dirty(false),
		sampleCount(0),
		texType(texType),
		format(TextureFormat::UNKNOWN),
		resUsage(Usage::NONE),
		mapUsage(Usage::NONE),
		size(0),
		handle(0),
		sync(nullptr),
		arraySize(0),
		mipLevels(0) {
		memset(&glTexInfo, 0, sizeof(glTexInfo));
	}

	BaseTexture::~BaseTexture() {
		releaseTex();
	}

	bool BaseTexture::_usageConflicted(Usage requiredUsage, Usage& preferredUsage, Usage conflictedUsage1, Usage conflictedUsage2) {
		using namespace srk::enum_operators;

		if ((requiredUsage & conflictedUsage1) != Usage::NONE) {
			if ((requiredUsage & conflictedUsage2) != Usage::NONE) {
				return false;
			} else {
				preferredUsage &= ~conflictedUsage2;
				return true;
			}
		} else if ((requiredUsage & conflictedUsage2) != Usage::NONE) {
			preferredUsage &= ~conflictedUsage1;
			return true;
		} else {
			preferredUsage &= ~conflictedUsage2;
			return true;
		}
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
			graphics.error("OpenGL Texture::create error : has not support requiredUsage " + String::toString((std::underlying_type_t<Usage>)u));
			return _createDone(graphics, false);
		}

		Usage allUsage;
		do {
			allUsage = requiredUsage | preferredUsage;

			auto cpuRead = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
			auto cpuWrite = (allUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE;

			if (cpuRead) {
				if (cpuWrite) {
					if (_usageConflicted(requiredUsage, preferredUsage, Usage::MAP_READ, Usage::MAP_WRITE)) {
						continue;
					} else {
						graphics.error("OpenGL Texture::create error : could not enable Usage::MAP_READ and Usage::MAP_WRITE at same time");
						return _createDone(graphics, false);
					}
				}

				if ((allUsage & Usage::UPDATE) == Usage::UPDATE) {
					if (_usageConflicted(requiredUsage, preferredUsage, Usage::MAP_READ, Usage::UPDATE)) {
						continue;
					} else {
						graphics.error("OpenGL Texture::create error : could not enable Usage::MAP_READ and Usage::UPDATE at same time");
						return _createDone(graphics, false);
					}
				}

				break;
			} else if (cpuWrite) {
				if ((allUsage & Usage::UPDATE) == Usage::UPDATE) {
					if (_usageConflicted(requiredUsage, preferredUsage, Usage::MAP_WRITE, Usage::UPDATE)) {
						continue;
					} else {
						graphics.error("OpenGL Texture::create error : could not enable Usage::MAP_WRITE and Usage::UPDATE at same time");
						return _createDone(graphics, false);
					}
				}

				break;
			} else {
				break;
			}
		} while (true);

		if (!sampleCount) sampleCount = 1;

		if (mipLevels) {
			if (sampleCount > 1) {
				graphics.error("OpenGL Texture::create error : could not enable multisampling and mipmap at same time");
				return _createDone(graphics, false);
			}

			auto maxLevels = TextureUtils::getMipLevels(dim.getMax());
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		} else {
			if (sampleCount > 1) {
				mipLevels = 1;
			} else {
				mipLevels = TextureUtils::getMipLevels(dim.getMax());
			}
		}

		auto isArray = arraySize && texType != TextureType::TEX3D;
		if (arraySize < 1 || texType == TextureType::TEX3D) arraySize = 1;

		this->dim.set(dim);

		glGenTextures(1, &handle);

		if (handle) {
			resUsage = requiredUsage | (preferredUsage & graphics.getTexCreateUsageMask());

			if (auto rst = Graphics::convertFormat(format); rst) {
				glTexInfo.internalFormat = rst->internalFormat;
				glTexInfo.format = rst->format;
				glTexInfo.type = rst->type;

				bool supportTexStorage = graphics.getInternalFeatures().supportTexStorage;

				auto perBlockBytes = TextureUtils::getPerBlockBytes(format);
				size_t mipsBytes;
				TextureUtils::getMipsInfo(format, dim, mipLevels, &mipsBytes);

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
								if (TextureUtils::isCompressedFormat(format)) {
									for (size_t i = 0; i < mipLevels; ++i) {
										auto size = TextureUtils::getBytes(format, w);
										for (size_t j = 0; j < arraySize; ++j) {
											if (auto subData = data[i + j * mipLevels]; subData) glCompressedTexSubImage2D(glTexInfo.target, i, 0, 0, w, j, glTexInfo.internalFormat, size, subData);
										}
										w = TextureUtils::getNextMipPixels(w);
									}
								} else {
									for (size_t i = 0; i < mipLevels; ++i) {
										for (size_t j = 0; j < arraySize; ++j) {
											if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage2D(glTexInfo.target, i, 0, 0, w, j, glTexInfo.format, glTexInfo.type, subData);
										}
										w = TextureUtils::getNextMipPixels(w);
									}
								}
							}
						} else {
							for (size_t i = 0; i < mipLevels; ++i) {
								for (size_t j = 0; j < arraySize; ++j) glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, w, j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] : nullptr);
								w = TextureUtils::getNextMipPixels(w);
							}
						}
					} else {
						glTexInfo.target = GL_TEXTURE_1D;
						glBindTexture(glTexInfo.target, handle);

						if (supportTexStorage) {
							glTexStorage1D(glTexInfo.target, mipLevels, glTexInfo.internalFormat, dim[0]);
							if (data) {
								if (TextureUtils::isCompressedFormat(format)) {
									for (size_t i = 0; i < mipLevels; ++i) {
										if (data[i]) glCompressedTexSubImage1D(glTexInfo.target, i, 0, w, glTexInfo.internalFormat, TextureUtils::getBytes(format, w), data[i]);
										w = TextureUtils::getNextMipPixels(w);
									}
								} else {
									for (size_t i = 0; i < mipLevels; ++i) {
										if (data[i]) glTexSubImage1D(glTexInfo.target, i, 0, w, glTexInfo.format, glTexInfo.type, data[i]);
										w = TextureUtils::getNextMipPixels(w);
									}
								}
							}
						} else {
							for (size_t i = 0; i < mipLevels; ++i) {
								glTexImage1D(glTexInfo.target, i, glTexInfo.internalFormat, w, 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
								w = TextureUtils::getNextMipPixels(w);
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
									if (TextureUtils::isCompressedFormat(format)) {
										for (size_t i = 0; i < mipLevels; ++i) {
											auto size = TextureUtils::getBytes(format, size2);
											for (size_t j = 0; j < arraySize; ++j) {
												if (auto subData = data[i + j * mipLevels]; subData) glCompressedTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size2[0], size2[1], j, glTexInfo.internalFormat, size, subData);
											}
											size2 = TextureUtils::getNextMipPixels(size2);
										}
									} else {
										for (size_t i = 0; i < mipLevels; ++i) {
											for (size_t j = 0; j < arraySize; ++j) {
												if (auto subData = data[i + j * mipLevels]; subData) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size2[0], size2[1], j, glTexInfo.format, glTexInfo.type, subData);
											}
											size2 = TextureUtils::getNextMipPixels(size2);
										}
									}
								}
							}
						} else {
							if (sampleCount > 1) {
								glTexImage3DMultisample(glTexInfo.target, sampleCount, glTexInfo.internalFormat, dim[0], dim[1], arraySize, true);
							} else {
								for (size_t i = 0; i < mipLevels; ++i) {
									for (size_t j = 0; j < arraySize; ++j) glTexImage3D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], j, 0, glTexInfo.format, glTexInfo.type, data ? data[i + j * mipLevels] : nullptr);
									size2 = TextureUtils::getNextMipPixels(size2);
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
									if (TextureUtils::isCompressedFormat(format)) {
										for (size_t i = 0; i < mipLevels; ++i) {
											if (data[i]) glCompressedTexSubImage2D(glTexInfo.target, i, 0, 0, size2[0], size2[1], glTexInfo.internalFormat, TextureUtils::getBytes(format, size2), data[i]);
											size2 = TextureUtils::getNextMipPixels(size2);
										}
									} else {
										for (size_t i = 0; i < mipLevels; ++i) {
											if (data[i]) glTexSubImage2D(glTexInfo.target, i, 0, 0, size2[0], size2[1], glTexInfo.format, glTexInfo.type, data[i]);
											size2 = TextureUtils::getNextMipPixels(size2);
										}
									}
								}
							}
						} else {
							if (sampleCount > 1) {
								glTexImage2DMultisample(glTexInfo.target, sampleCount, glTexInfo.internalFormat, dim[0], dim[1], true);
							} else {
								for (size_t i = 0; i < mipLevels; ++i) {
									glTexImage2D(glTexInfo.target, i, glTexInfo.internalFormat, size2[0], size2[1], 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
									size2 = TextureUtils::getNextMipPixels(size2);

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
							if (TextureUtils::isCompressedFormat(format)) {
								for (uint32_t i = 0; i < mipLevels; ++i) {
									if (data[i]) glCompressedTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size3[0], size3[1], size3[2], glTexInfo.internalFormat, TextureUtils::getBytes(format, size3.cast<2>()) * size3[2], data[i]);
									size3 = TextureUtils::getNextMipPixels(size3);
								}
							} else {
								for (uint32_t i = 0; i < mipLevels; ++i) {
									if (data[i]) glTexSubImage3D(glTexInfo.target, i, 0, 0, 0, size3[0], size3[1], size3[2], glTexInfo.format, glTexInfo.type, data[i]);
									size3 = TextureUtils::getNextMipPixels(size3);
								}
							}
						}
					} else {
						for (uint32_t i = 0; i < mipLevels; ++i) {
							glTexImage3D(glTexInfo.target, i, glTexInfo.internalFormat, size3[0], size3[1], size3[2], 0, glTexInfo.format, glTexInfo.type, data ? data[i] : nullptr);
							size3 = TextureUtils::getNextMipPixels(size3);
						}
					}

					break;
				}
				default:
					break;
				}

				if (sampleCount == 1) glTexParameteri(glTexInfo.target, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
				glBindTexture(glTexInfo.target, 0);

				if ((resUsage & Usage::MAP_READ_WRITE) != Usage::NONE) {
					mapData.resize(mipLevels* arraySize);
					Vec3uz size3(dim);
					for (decltype(mipLevels) i = 0; i < mipLevels; ++i) {
						auto& md = mapData[i];
						md.size = TextureUtils::getBlocks(format, size3).getMultiplies() * perBlockBytes;
						md.usage = Usage::NONE;
						md.inited = false;
						md.buffer = nullptr;
						md.arraySlice = 0;
						md.mipSlice = i;
						md.range.size = size3;

						for (decltype(arraySize) j = 1; j < arraySize; ++j) {
							auto& md1 = mapData[calcSubresource(i, j, mipLevels)];
							md1.size = md.size;
							md1.usage = Usage::NONE;
							md.inited = false;
							md1.buffer = nullptr;
							md1.arraySlice = j;
							md1.mipSlice = i;
							md1.range.size = size3;
						}

						size3 = TextureUtils::getNextMipPixels(size3);
					}
				}

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
		using namespace srk::enum_operators;

		auto ret = Usage::NONE;
		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) {
			auto usage = expectMapUsage & resUsage & Usage::MAP_READ_WRITE;

			if (usage == Usage::NONE) {
				_unmap(subresource, false);
			} else {
				auto& md = mapData[subresource];
				ret = usage;

				if (md.usage != usage) {
					_unmap(md, false);
					md.usage = usage;
				}
			}
		}

		return ret;
	}

	void BaseTexture::unmap(size_t arraySlice, size_t mipSlice) {
		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) _unmap(mapData[subresource], false);
	}

	void BaseTexture::_unmap(MapData& md, bool discard) {
		using namespace srk::enum_operators;

		if (md.usage != Usage::NONE) {
			if (md.buffer) {
				md.buffer->unmap();
				if (md.inited && (md.usage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
					glBindBuffer(md.buffer->getBufferType(), md.buffer->getHandle());
					auto rst = _update(md.arraySlice, md.mipSlice, md.range, nullptr);
					glBindBuffer(md.buffer->getBufferType(), 0);
				}
				delete md.buffer;
				md.buffer = nullptr;
			}
			
			md.usage = Usage::NONE;
			md.inited = false;
		}
	}

	size_t BaseTexture::read(Graphics& graphics, size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		using namespace srk::enum_operators;

		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) {
			auto& md = mapData[subresource];
			if ((md.usage & Usage::MAP_READ) == Usage::MAP_READ) {
				if (!dst || !dstLen || offset >= md.size) return 0;

				if (!md.inited) {
					if (!md.buffer) {
						ScopePtr buffer = new PixelBuffer();
						if (!buffer->create(graphics, md.size, Usage::MAP_READ, Usage::NONE)) return -1;
						md.buffer = buffer;
						buffer.reset<false>();
					}
					if (!md.buffer->copyFrom(mipSlice, this)) return -1;

					md.buffer->map(Usage::MAP_READ);
					md.inited = true;
				}

				auto readLen = std::min<size_t>(md.size - offset, dstLen);
				return md.buffer->read(dst, readLen, offset);
			}
		}
		return -1;
	}

	size_t BaseTexture::write(Graphics& graphics, size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		using namespace srk::enum_operators;

		if (auto subresource = calcSubresource(mipSlice, arraySlice, mipLevels); subresource < mapData.size()) {
			auto& md = mapData[subresource];
			if ((md.usage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
				if (!data || !length || offset >= md.size) return 0;

				if (!md.inited) {
					if (!md.buffer) {
						ScopePtr<PixelBuffer> readBuffer;
						if (offset || length < md.size) {
							readBuffer = new PixelBuffer();
							if (!readBuffer->create(graphics, md.size, Usage::MAP_READ, Usage::NONE)) return -1;
							if (!readBuffer->copyFrom(mipSlice, this)) return -1;
						}

						ScopePtr buffer = new PixelBuffer();
						if (!buffer->create(graphics, md.size, Usage::MAP_WRITE, Usage::NONE)) return -1;
						if (readBuffer) buffer->copyFrom(graphics, readBuffer);

						md.buffer = buffer;
						buffer.reset<false>();
					}

					md.buffer->map(Usage::MAP_WRITE);
					md.inited = true;
				}

				length = std::min<size_t>(length, md.size - offset);
				return md.buffer->write(data, length, offset);
			}
		}
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
		using namespace srk::enum_operators;

		if (dstArraySlice >= internalArraySize || dstMipSlice >= mipLevels || !src || src->getGraphics() != graphics) return false;

		auto srcBase = (BaseTexture*)src->getNative();
		if (srcArraySlice >= srcBase->internalArraySize || srcMipSlice >= srcBase->mipLevels) return false;

		if ((srcBase->resUsage & Usage::COPY_SRC) == Usage::NONE) {
			graphics.error("OpenGL Texture::copyFrom error : no Usage::COPY_SRC");
			return false;
		}

		if ((resUsage & Usage::COPY_DST) == Usage::NONE) {
			graphics.error("OpenGL Texture::copyFrom error : no Usage::COPY_DST");
			return false;
		}

		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		switch (srcBase->texType) {
		case TextureType::TEX1D:
			glFramebufferTexture1D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcBase->glTexInfo.target, srcBase->handle, srcMipSlice);
			break;
		case TextureType::TEX2D:
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcBase->glTexInfo.target, srcBase->handle, srcMipSlice);
			break;
		case TextureType::TEX3D:
			glFramebufferTexture3D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcBase->glTexInfo.target, srcBase->handle, srcMipSlice, srcRange.pos[2]);
			break;
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
				glCopyTexSubImage2D(GL_TEXTURE_2D, dstMipSlice, dstPos[0], dstPos[1], srcRange.pos[0], srcRange.pos[1], srcRange.size[0], srcRange.size[1]);
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

	/*bool BaseTexture::copyFrom(Graphics& graphics, size_t arraySlice, size_t mipSlice, const Box3uz& range, const IPixelBuffer* pixelBuffer) {
		if (!pixelBuffer || graphics != pixelBuffer->getGraphics()) return false;

		auto native = (BaseBuffer*)pixelBuffer->getNative();
		if (!native) return false;

		if (!native->handle || native->bufferType != GL_PIXEL_UNPACK_BUFFER) return false;

		glBindBuffer(native->bufferType, native->handle);
		auto rst = _update(arraySlice, mipSlice, range, nullptr);
		glBindBuffer(native->bufferType, 0);
		native->doSync<true>();

		return rst;
	}*/

	void BaseTexture::flush() {
		if (dirty) {
			waitServerSync();
			sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			dirty = false;
		}
	}

	void BaseTexture::releaseTex() {
		if (handle) {
			if (!mapData.empty()) {
				for (auto& md : mapData) _unmap(md, true);
				mapData.clear();
			}

			releaseSync();

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
		{
			if (TextureUtils::isCompressedFormat(format)) {
				glCompressedTexSubImage1D(glTexInfo.target, mipSlice, range.pos[0], range.size[0], glTexInfo.internalFormat, TextureUtils::getBytes(format, range.size[0]), data);
			} else {
				glTexSubImage1D(glTexInfo.target, mipSlice, range.pos[0], range.size[0], glTexInfo.format, glTexInfo.type, data);
			}

			break;
		}
		case GL_TEXTURE_1D_ARRAY:
		{
			if (TextureUtils::isCompressedFormat(format)) {
				glCompressedTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], 0, range.size[0], arraySlice, glTexInfo.internalFormat, TextureUtils::getBytes(format, range.size[0]), data);
			} else {
				glTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], 0, range.size[0], arraySlice, glTexInfo.format, glTexInfo.type, data);
			}

			break;
		}
		case GL_TEXTURE_2D:
		{
			if (TextureUtils::isCompressedFormat(format)) {
				glCompressedTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.size[0], range.size[1], glTexInfo.internalFormat, TextureUtils::getBytes(format, range.size.cast<2>()), data);
			} else {
				glTexSubImage2D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.size[0], range.size[1], glTexInfo.format, glTexInfo.type, data);
			}

			break;
		}
		case GL_TEXTURE_2D_ARRAY:
		{
			if (TextureUtils::isCompressedFormat(format)) {
				glCompressedTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], 0, range.size[0], range.size[1], arraySlice, glTexInfo.internalFormat, TextureUtils::getBytes(format, range.size.cast<2>()), data);
			} else {
				glTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], 0, range.size[0], range.size[1], arraySlice, glTexInfo.format, glTexInfo.type, data);
			}

			break;
		}
		case GL_TEXTURE_3D:
		{
			if (TextureUtils::isCompressedFormat(format)) {
				glCompressedTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.pos[2], range.size[0], range.size[1], range.size[2], glTexInfo.internalFormat, TextureUtils::getBytes(format, range.size.cast<2>()) * range.size[2], data);
			} else {
				glTexSubImage3D(glTexInfo.target, mipSlice, range.pos[0], range.pos[1], range.pos[2], range.size[0], range.size[1], range.size[2], glTexInfo.format, glTexInfo.type, data);
			}

			break;
		}
		default:
			glBindTexture(glTexInfo.target, 0);
			return false;
		}

		glBindTexture(glTexInfo.target, 0);
		return true;
	}
}