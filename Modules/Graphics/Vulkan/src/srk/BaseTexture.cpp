#include "BaseTexture.h"
#include "BaseBuffer.h"
#include "srk/StringUtility.h"

namespace srk::modules::graphics::vulkan {
	BaseTexture::BaseTexture(TextureType texType) :
		_sampleCount(0),
		_texType(texType),
		_format(TextureFormat::UNKNOWN),
		_vkFlags(0),
		_memAllocator(nullptr),
		_image(nullptr),
		_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED),
		_allocation(nullptr),
		_mapCount(0),
		_size(0),
		_memAlignment(0),
		_mipLevels(0),
		_arraySize(0),
		_usage(Usage::NONE),
		_internalUsage(Usage::NONE),
		_mappedData(nullptr) {
	}

	BaseTexture::~BaseTexture() {
		destroy();
	}

	bool BaseTexture::create(Graphics& graphics, const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount,
		TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data) {
		using namespace srk::enum_operators;

		destroy();

		requiredUsage &= Usage::TEXTURE_RESOURCE_CREATE_ALL;
		preferredUsage &= Usage::TEXTURE_RESOURCE_CREATE_ALL;
		auto supportedUsages = graphics.getTexCreateUsageMask();
		if (auto u = (requiredUsage & (~graphics.getTexCreateUsageMask())); u != Usage::NONE) {
			graphics.error("Vulkan BaseTexture::create error : has not support requiredUsage " + StringUtility::toString(std::to_underlying(u)));
			return false;
		}

		if (!sampleCount) sampleCount = 1;

		if (mipLevels) {
			if (sampleCount > 1) {
				graphics.error("Vulkan BaseTexture::create error : could not enable multisampling and mipmap at same time");
				destroy();
				return false;
			}

			auto maxLevels = TextureUtility::getMipLevels(dim);
			if (mipLevels > maxLevels) mipLevels = maxLevels;
		} else {
			if (sampleCount > 1) {
				mipLevels = 1;
			} else {
				mipLevels = TextureUtility::getMipLevels(dim);
			}
		}

		auto allUsage = requiredUsage | preferredUsage;
		_usage = Usage::NONE;

		auto isArray = arraySize && _texType != TextureType::TEX3D;
		if (arraySize < 1 || _texType == TextureType::TEX3D) arraySize = 1;

		auto cpuRead = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
		auto cpuWriteUsage = allUsage & Usage::MAP_WRITE_UPDATE;
		auto cpuWrite = cpuWriteUsage != Usage::NONE;

		VkImageCreateInfo imageCreateInfo;
		memset(&imageCreateInfo, 0, sizeof(imageCreateInfo));
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = Graphics::convertTextureType(_texType);
		imageCreateInfo.format = Graphics::convertTextureFormat(format);
		if (imageCreateInfo.format == VK_FORMAT_UNDEFINED) {
			destroy();
			return false;
		}
		imageCreateInfo.extent.width = dim[0];
		imageCreateInfo.extent.height = dim[1];
		imageCreateInfo.extent.depth = dim[2];
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = arraySize;
		imageCreateInfo.samples = Graphics::convertSampleCount(sampleCount);
		if (imageCreateInfo.samples == 0) {
			destroy();
			return false;
		}
		imageCreateInfo.tiling = cpuRead || cpuWrite ?  VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
		if ((allUsage & Usage::RENDERABLE) == Usage::RENDERABLE) {
			_usage |= Usage::RENDERABLE;
			imageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if ((allUsage & Usage::COPY_SRC) == Usage::COPY_SRC) {
			_usage |= Usage::COPY_SRC;
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if ((allUsage & Usage::COPY_DST) == Usage::COPY_DST) {
			_usage |= Usage::COPY_DST;
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0;//
		imageCreateInfo.pQueueFamilyIndices = nullptr;//
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		auto initData = data != nullptr;
		
		_internalUsage = Usage::NONE;
		if (initData && !cpuWrite) {
			_internalUsage |= Usage::COPY_DST;
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VmaMemoryUsage vmaUsage;
		if (cpuRead) {
			_usage |= Usage::MAP_READ;
			if (cpuWrite) {
				_usage |= cpuWriteUsage;
				vmaUsage = VMA_MEMORY_USAGE_CPU_ONLY;

			} else {
				vmaUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
			}
		} else if (cpuWrite) {
			_usage |= cpuWriteUsage;
			vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		} else {
			vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		}

		if ((_usage & requiredUsage) != requiredUsage) {
			graphics.error("Vulkan BaseTexture::create error : has not support requiredUsage " + StringUtility::toString(std::to_underlying(requiredUsage & (~(_usage & requiredUsage)))));
			destroy();
			return false;
		}

		if (imageCreateInfo.usage == 0) {
			_internalUsage |= Usage::RENDERABLE;
			imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		}

		VkImageFormatProperties imageFormatProperties;
		if (vkGetPhysicalDeviceImageFormatProperties(graphics.getVkPhysicalDevice(), imageCreateInfo.format, imageCreateInfo.imageType, imageCreateInfo.tiling, imageCreateInfo.usage, imageCreateInfo.flags, &imageFormatProperties) != VK_SUCCESS) {
			destroy();
			return false;
		}

		if (imageCreateInfo.extent.width > imageFormatProperties.maxExtent.width) {
			graphics.error("Vulkan BaseTexture::create error : width(" + StringUtility::toString(imageCreateInfo.extent.width) + ") > maxWidth(" + StringUtility::toString(imageFormatProperties.maxExtent.width) + ")");
			destroy();
			return false;
		}

		if (imageCreateInfo.extent.height > imageFormatProperties.maxExtent.height) {
			graphics.error("Vulkan BaseTexture::create error : height(" + StringUtility::toString(imageCreateInfo.extent.height) + ") > maxHeight(" + StringUtility::toString(imageFormatProperties.maxExtent.height) + ")");
			destroy();
			return false;
		}

		if (imageCreateInfo.extent.depth > imageFormatProperties.maxExtent.depth) {
			graphics.error("Vulkan BaseTexture::create error : depth(" + StringUtility::toString(imageCreateInfo.extent.depth) + ") > maxDepth(" + StringUtility::toString(imageFormatProperties.maxExtent.depth) + ")");
			destroy();
			return false;
		}

		if (imageCreateInfo.mipLevels > imageFormatProperties.maxMipLevels) {
			graphics.error("Vulkan BaseTexture::create error : mipLevels(" + StringUtility::toString(imageCreateInfo.mipLevels) + ") > maxMipLevels(" + StringUtility::toString(imageFormatProperties.maxMipLevels) + ")");
			destroy();
			return false;
		}

		if (imageCreateInfo.arrayLayers > imageFormatProperties.maxArrayLayers) {
			graphics.error("Vulkan BaseTexture::create error : arrayLayers(" + StringUtility::toString(imageCreateInfo.arrayLayers) + ") > maxArrayLayers(" + StringUtility::toString(imageFormatProperties.maxArrayLayers) + ")");
			destroy();
			return false;
		}

		if (imageCreateInfo.samples > imageFormatProperties.sampleCounts) {
			graphics.error("Vulkan BaseTexture::create error : samples(" + StringUtility::toString((size_t)imageCreateInfo.samples) + ") > sampleCounts(" + StringUtility::toString((size_t)imageFormatProperties.sampleCounts) + ")");
			destroy();
			return false;
		}

		/*if (imageCreateInfo.extent.width > imageFormatProperties.maxExtent.width || imageCreateInfo.extent.height > imageFormatProperties.maxExtent.height || imageCreateInfo.extent.depth > imageFormatProperties.maxExtent.depth || 
			imageCreateInfo.mipLevels > imageFormatProperties.maxMipLevels || imageCreateInfo.arrayLayers > imageFormatProperties.maxArrayLayers || imageCreateInfo.samples > imageFormatProperties.sampleCounts) {
			destroy();
			return false;
		}*/

		VmaAllocationCreateInfo allocationCreateInfo;
		memset(&allocationCreateInfo, 0, sizeof(allocationCreateInfo));
		allocationCreateInfo.usage = vmaUsage;

		_memAllocator = graphics.getMemAllocator();
		_internalUsage |= _usage;
		if ((_internalUsage & Usage::UPDATE) == Usage::UPDATE) _internalUsage |= Usage::MAP_WRITE;

		if (vmaCreateImage(_memAllocator, &imageCreateInfo, &allocationCreateInfo, &_image, &_allocation, nullptr) != VK_SUCCESS) {
			destroy();
			return false;
		}

		vmaGetAllocationMemoryProperties(_memAllocator, _allocation, &_vkFlags);

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(graphics.getVkDevice(), _image, &memReq);
		_memAlignment = memReq.alignment;

		size_t mipsBytes;
		std::vector<size_t> mipBytesArr(initData ? mipLevels : 0);
		auto hasMipDimArr = initData || (_internalUsage & Usage::MAP_READ_WRITE) != Usage::NONE;
		std::vector<Vec3uz> mipDimArr(hasMipDimArr ? mipLevels : 0);
		TextureUtility::getMipsInfo(format, dim, mipLevels, &mipsBytes, initData ? mipBytesArr.data() : nullptr, hasMipDimArr ? mipDimArr.data() : nullptr);
		_size = mipsBytes * arraySize;

		if (initData) {
			auto perBlockBytes = TextureUtility::getPerBlockBytes(format);

			if (_vkFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
				void* dst;
				if (_map(dst)) {
					size_t offset = 0;
					for (size_t i = 0; i < arraySize; ++i) {
						for (size_t j = 0; j < mipLevels; ++j) {
							auto src = (const uint8_t*)data[calcSubresource(j, i, mipLevels)];
							auto rowBytes = TextureUtility::getBlocks(format, mipDimArr[j][0]) * perBlockBytes;

							if (rowBytes % _memAlignment){
								auto remainMipBytes = mipBytesArr[j];
								auto div = std::div((std::make_signed_t<decltype(offset)>)rowBytes, (std::make_signed_t<decltype(_memAlignment)>)_memAlignment);
								auto alignBytes = div.quot * _memAlignment;
								if (div.rem) alignBytes += _memAlignment;

								do {
									memcpy((uint8_t*)dst + offset, src, rowBytes);

									src += rowBytes;
									offset += alignBytes;
									remainMipBytes -= rowBytes;
								} while (remainMipBytes);
							} else {
								memcpy((uint8_t*)dst + offset, src, mipBytesArr[j]);
								offset += mipBytesArr[j];
							}
						}
					}
					_unmap();
				} else {
					destroy();
					return false;
				}
				
			} else {
				BaseBuffer src;
				if (mipLevels == 1 && arraySize == 1) {
					if (!src.create(graphics, _size, Usage::COPY_SRC, Usage::NONE, data[0], _size)) {
						destroy();
						return false;
					}
				} else {
					if (!src.create(graphics, _size, Usage::MAP_WRITE | Usage::COPY_SRC, Usage::NONE, nullptr, 0)) {
						destroy();
						return false;
					}

					if (src.map(Usage::MAP_WRITE) != Usage::MAP_WRITE) {
						destroy();
						return false;
					}
					for (size_t i = 0; i < arraySize; ++i) {
						auto bufferOffset = mipsBytes * i;
						for (size_t j = 0; j < mipLevels; ++j) {
							if (src.write(data[calcSubresource(j, i, mipLevels)], mipBytesArr[j], bufferOffset) != mipBytesArr[j]) {
								destroy();
								return false;
							}
							bufferOffset += mipBytesArr[j];
						}
					}
					src.unmap();
				}

				auto cmd = graphics.beginOneTimeCommands();
				if (!cmd) {
					destroy();
					return false;
				}

				VkImageSubresourceRange range;
				range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				range.baseMipLevel = 0;
				range.levelCount = mipLevels;
				range.baseArrayLayer = 0;
				range.layerCount = arraySize;
				Graphics::transitionImageLayout(cmd.getVkCommandBuffer(), _image, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);

				std::vector<VkBufferImageCopy> bufferImageCopies(mipLevels * arraySize);
				size_t idx = 0;
				for (size_t i = 0; i < arraySize; ++i) {
					auto bufferOffset = mipsBytes * i;
					for (size_t j = 0; j < mipLevels; ++j) {
						auto& bufferImageCopy = bufferImageCopies[idx++];
						bufferImageCopy.bufferOffset = bufferOffset;
						bufferImageCopy.bufferRowLength = 0;
						bufferImageCopy.bufferImageHeight = 0;
						bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						bufferImageCopy.imageSubresource.mipLevel = j;
						bufferImageCopy.imageSubresource.baseArrayLayer = i;
						bufferImageCopy.imageSubresource.layerCount = 1;
						bufferImageCopy.imageOffset = { 0, 0, 0 };
						const auto& dim = mipDimArr[j];
						bufferImageCopy.imageExtent = { (uint32_t)dim[0], (uint32_t)dim[1], (uint32_t)dim[2] };

						bufferOffset += mipBytesArr[j];
					}
				}

				vkCmdCopyBufferToImage(cmd.getVkCommandBuffer(), src.getVkBuffer(), _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bufferImageCopies.size(), bufferImageCopies.data());
				_tryRestoreImageLayout(cmd.getVkCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);

				if (!graphics.endOneTimeCommands(cmd)) {
					destroy();
					return false;
				}
			}
		}

		if ((_internalUsage & Usage::MAP_READ_WRITE) != Usage::NONE) {
			_mapData.resize(mipLevels * arraySize);
			auto perBlockBytes = TextureUtility::getPerBlockBytes(format);

			size_t offset = 0;
			for (size_t i = 0; i < arraySize; ++i) {
				for (size_t j = 0; j < mipLevels; ++j) {
					auto blocks = TextureUtility::getBlocks(format, mipDimArr[j]);

					auto& md = _mapData[calcSubresource(i, j, mipLevels)];
					md.size = TextureUtility::getBlocks(format, mipDimArr[j]).getMultiplies() * perBlockBytes * dim[2];
					md.usage = Usage::NONE;
					md.begin = offset;
					md.rowBytes = blocks[0] * perBlockBytes;
					md.rowAlignSize = ((md.rowBytes + _memAlignment - 1) / _memAlignment) * _memAlignment;
					md.sliceAlignSize = md.rowAlignSize * blocks[1];
					md.completelyAlign = (md.rowBytes % _memAlignment) == 0;

					offset += md.sliceAlignSize * mipDimArr[j][2];
				}
			}
		}

		_dim = dim;
		_format = format;
		_mipLevels = mipLevels;
		_arraySize = arraySize;

		return true;
	}

	Usage BaseTexture::map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) {
		using namespace srk::enum_operators;

		auto ret = Usage::NONE;
		if (auto subresource = calcSubresource(mipSlice, arraySlice, _mipLevels); subresource < _mapData.size()) {
			auto usage = expectMapUsage & _internalUsage & Usage::MAP_READ_WRITE;

			if (usage == Usage::NONE) {
				_unmap(subresource);
			} else {
				auto& md = _mapData[subresource];
				ret = usage;

				if (md.usage != usage) {
					if (_mappedData) {
						md.usage = usage;
					} else {
						if (_map(_mappedData)) {
							md.usage = usage;
							++_mapCount;
						} else {
							_unmap(md);
							ret = Usage::NONE;
						}
					}
				}
			}
		}
		
		return ret;
	}

	bool BaseTexture::_map(void*& mappedData) {
		if (vmaMapMemory(_memAllocator, _allocation, &mappedData) != VK_SUCCESS) return false;
		if (!(_vkFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) vmaInvalidateAllocation(_memAllocator, _allocation, 0, _size);
		return true;
	}

	void BaseTexture::unmap(size_t arraySlice, size_t mipSlice) {
		if (auto subresource = calcSubresource(mipSlice, arraySlice, _mipLevels); subresource < _mapData.size()) _unmap(subresource);
	}

	void BaseTexture::_unmap() {
		if (!(_vkFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) vmaFlushAllocation(_memAllocator, _allocation, 0, _size);
		vmaUnmapMemory(_memAllocator, _allocation);
	}

	void BaseTexture::_unmap(MapData& md) {
		if (md.usage != Usage::NONE) {
			md.usage = Usage::NONE;
			if (--_mapCount == 0) {
				_unmap();
				_mappedData = nullptr;
			}
		}
	}

	size_t BaseTexture::read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) {
		using namespace srk::enum_operators;

		if (auto subresource = calcSubresource(mipSlice, arraySlice, _mipLevels); subresource < _mapData.size()) {
			auto& md = _mapData[subresource];
			if ((md.usage & Usage::MAP_READ) == Usage::MAP_READ) {
				if (!dst || !dstLen || offset >= md.size) return 0;

				auto readLen = std::min<size_t>(md.size - offset, dstLen);

				if (md.completelyAlign) {
					memcpy(dst, (const uint8_t*)_mappedData + md.begin + offset, readLen);
				} else {
					auto remainBytes = readLen;
					size_t srcOffset = md.begin;
					size_t dstOffset = 0;
					if (offset) {
						auto skipRows = offset / md.rowBytes;
						srcOffset += md.rowAlignSize * skipRows;
						offset -= skipRows * md.rowBytes;

						if (offset) {
							auto remainRowBytes = std::min(md.rowBytes - offset, remainBytes);
							memcpy(dst, (const uint8_t*)_mappedData + srcOffset + offset, remainRowBytes);
							dstOffset += remainRowBytes;
							remainBytes -= remainRowBytes;
							srcOffset += md.rowAlignSize;
						}
					}

					while (remainBytes) {
						auto remainRowBytes = std::min(md.rowBytes, remainBytes);
						memcpy((uint8_t*)dst + dstOffset, (const uint8_t*)_mappedData + srcOffset, remainRowBytes);
						dstOffset += remainRowBytes;
						remainBytes -= remainRowBytes;
						srcOffset += md.rowAlignSize;
					}
				}

				return readLen;
			}
		}
		return -1;
	}

	size_t BaseTexture::write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) {
		using namespace srk::enum_operators;

		if (auto subresource = calcSubresource(mipSlice, arraySlice, _mipLevels); subresource < _mapData.size()) {
			auto& md = _mapData[subresource];
			if ((md.usage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
				if (!data || !length || offset >= md.size) return 0;
				
				length = std::min<size_t>(length, md.size - offset);
				
				if (md.completelyAlign) {
					memcpy((uint8_t*)_mappedData + md.begin + offset, data, length);
				} else {
					auto remainBytes = length;
					size_t srcOffset = 0;
					size_t dstOffset = md.begin;
					if (offset) {
						auto skipRows = offset / md.rowBytes;
						dstOffset += md.rowAlignSize * skipRows;
						offset -= skipRows * md.rowBytes;

						if (offset) {
							auto remainRowBytes = std::min(md.rowBytes - offset, remainBytes);
							memcpy((uint8_t*)_mappedData + dstOffset + offset, data, remainRowBytes);
							srcOffset += remainRowBytes;
							remainBytes -= remainRowBytes;
							dstOffset += md.rowAlignSize;
						}
					}

					while (remainBytes) {
						auto remainRowBytes = std::min(md.rowBytes, remainBytes);
						memcpy((uint8_t*)_mappedData + dstOffset, (const uint8_t*)data + srcOffset, remainRowBytes);
						srcOffset += remainRowBytes;
						remainBytes -= remainRowBytes;
						dstOffset += md.rowAlignSize;
					}
				}

				return length;
			}
		}
		return -1;
	}

	bool BaseTexture::update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data) {
		using namespace srk::enum_operators;

		if (data && (_internalUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (auto subresource = calcSubresource(mipSlice, arraySlice, _mipLevels); subresource < _mapData.size()) {
				auto& md = _mapData[subresource];

				if ((md.usage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
					_write(md, range, data);
				} else {
					auto old = md.usage;
					if (map(arraySlice, mipSlice, Usage::MAP_WRITE) == Usage::NONE) {
						map(arraySlice, mipSlice, old);
						return false;
					}
					
					_write(md, range, data);
					map(arraySlice, mipSlice, old);
				}
				return true;
			}
		}
		return false;
	}

	void BaseTexture::_write(const MapData& md, const Box3uz& range, const void* data) {
		auto skipBlocks = TextureUtility::getBlocks(_format, range.pos.cast<2>());
		auto sliceWriteBlocks = TextureUtility::getBlocks(_format, range.size.cast<2>());
		auto sliceWriteBytes = TextureUtility::getBytes(_format, sliceWriteBlocks.getMultiplies());

		size_t dstOffset = md.begin + md.sliceAlignSize * range.pos[2] + skipBlocks[1] * md.rowBytes;
		if (md.completelyAlign && skipBlocks[0]) {
			for (size_t i = 0; i < range.size[2]; ++i) {
				memcpy((uint8_t*)_mappedData + dstOffset, (const uint8_t*)data + i * sliceWriteBytes, sliceWriteBytes);
				dstOffset += md.sliceAlignSize;
			}
		} else {
			dstOffset += TextureUtility::getBytes(_format, skipBlocks[0]);
			const auto rowBytes = TextureUtility::getBytes(_format, sliceWriteBlocks[0]);
			for (size_t i = 0; i < range.size[2]; ++i) {
				auto srcOffset = 0;
				for (size_t j = 0; j < sliceWriteBlocks[1]; ++j) memcpy((uint8_t*)_mappedData + dstOffset + j * md.rowBytes, (const uint8_t*)data + srcOffset + j * rowBytes, rowBytes);

				dstOffset += md.sliceAlignSize;
				srcOffset += sliceWriteBytes;
			}
		}
	}

	bool BaseTexture::copyFrom(Graphics& graphics, const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) {
		using namespace srk::enum_operators;

		if (dstArraySlice >= _arraySize || dstMipSlice >= _mipLevels || !src || src->getGraphics() != graphics) return false;

		auto srcBase = (BaseTexture*)src->getNative();
		if (srcArraySlice >= srcBase->_arraySize || srcMipSlice >= srcBase->_mipLevels) return false;

		if ((srcBase->_usage & Usage::COPY_SRC) == Usage::NONE) {
			graphics.error("OpenGL Texture::copyFrom error : no Usage::COPY_SRC");
			return false;
		}

		if ((_usage & Usage::COPY_DST) == Usage::NONE) {
			graphics.error("OpenGL Texture::copyFrom error : no Usage::COPY_DST");
			return false;
		}

		auto cmd = graphics.beginOneTimeCommands();
		if (!cmd) return false;

		VkImageSubresourceRange vkSrcRange;
		vkSrcRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkSrcRange.baseMipLevel = srcMipSlice;
		vkSrcRange.levelCount = 1;
		vkSrcRange.baseArrayLayer = srcArraySlice;
		vkSrcRange.layerCount = 1;
		
		VkImageSubresourceRange vkDstRange;
		vkDstRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkDstRange.baseMipLevel = dstMipSlice;
		vkDstRange.levelCount = 1;
		vkDstRange.baseArrayLayer = dstArraySlice;
		vkDstRange.layerCount = 1;

		VkImageCopy imageCopy;
		imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopy.srcSubresource.mipLevel = srcMipSlice;
		imageCopy.srcSubresource.baseArrayLayer = srcArraySlice;
		imageCopy.srcSubresource.layerCount = 1;
		imageCopy.srcOffset.x = srcRange.pos[0];
		imageCopy.srcOffset.y = srcRange.pos[1];
		imageCopy.srcOffset.z = srcRange.pos[2];
		imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopy.dstSubresource.mipLevel = dstMipSlice;
		imageCopy.dstSubresource.baseArrayLayer = dstArraySlice;
		imageCopy.dstSubresource.layerCount = 1;
		imageCopy.dstOffset.x = dstPos[0];
		imageCopy.dstOffset.y = dstPos[1];
		imageCopy.dstOffset.z = dstPos[2];
		imageCopy.extent.width = srcRange.size[0];
		imageCopy.extent.height = srcRange.size[1];
		imageCopy.extent.depth = srcRange.size[2];
		Graphics::transitionImageLayout(cmd.getVkCommandBuffer(), srcBase->_image, srcBase->_imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkSrcRange);
		Graphics::transitionImageLayout(cmd.getVkCommandBuffer(), _image, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkDstRange);
		vkCmdCopyImage(cmd.getVkCommandBuffer(), srcBase->_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
		srcBase->_tryRestoreImageLayout(cmd.getVkCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkSrcRange);
		_tryRestoreImageLayout(cmd.getVkCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkDstRange);
		
		return graphics.endOneTimeCommands(cmd);
	}

	void BaseTexture::destroy() {
		if (_mappedData) {
			_unmap();
			_mapCount = 0;
		}

		if (_image) {
			vmaDestroyImage(_memAllocator, _image, _allocation);
			_image = nullptr;
		}

		_sampleCount = 0;
		_dim = 0;

		_memAllocator = nullptr;
		_allocation = nullptr;
		_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		_size = 0;
		_memAlignment = 0;
		_mipLevels = 0;
		_arraySize = 0;
		_usage = Usage::NONE;
		_internalUsage = Usage::NONE;
		_mappedData = nullptr;
		_vkFlags = 0;
		_mapData.clear();
	}

	void BaseTexture::_tryRestoreImageLayout(VkCommandBuffer cmd, VkImageLayout cur, const VkImageSubresourceRange& range) {
		if (_imageLayout == VK_IMAGE_LAYOUT_UNDEFINED || _imageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
			_imageLayout = cur;
		} else {
			Graphics::transitionImageLayout(cmd, _image, cur, _imageLayout, range);
		}
	}
}