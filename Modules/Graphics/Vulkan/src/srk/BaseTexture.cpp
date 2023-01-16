#include "BaseTexture.h"
#include "BaseBuffer.h"
#include "srk/Image.h"

namespace srk::modules::graphics::vulkan {
	BaseTexture::BaseTexture(TextureType texType) :
		_sampleCount(0),
		_texType(texType),
		_format(TextureFormat::UNKNOWN),
		_vkFlags(0),
		_memAllocator(nullptr),
		_image(nullptr),
		_allocation(nullptr),
		_size(0),
		_usage(Usage::NONE),
		_internalUsage(Usage::NONE),
		_mapUsage(Usage::NONE),
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
			graphics.error(std::format("Vulkan BaseTexture::create error : has not support Usage {}", (std::underlying_type_t<Usage>)u));
			return false;
		}

		if (!sampleCount) sampleCount = 1;

		if (mipLevels) {
			if (sampleCount > 1) {
				graphics.error("Vulkan BaseTexture::create error : could not enable multisampling and mipmap at same time");
				destroy();
				return false;
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
			graphics.error(std::format("Vulkan BaseTexture::create error : has not support preferredUsage {}", (std::underlying_type_t<Usage>)(requiredUsage & (~(_usage & requiredUsage)))));
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

		if (imageCreateInfo.extent.width > imageFormatProperties.maxExtent.width || imageCreateInfo.extent.height > imageFormatProperties.maxExtent.height || imageCreateInfo.extent.depth > imageFormatProperties.maxExtent.depth || 
			imageCreateInfo.mipLevels > imageFormatProperties.maxMipLevels || imageCreateInfo.arrayLayers > imageFormatProperties.maxArrayLayers || imageCreateInfo.samples > imageFormatProperties.sampleCounts) {
			destroy();
			return false;
		}

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

		size_t mipsBytes;
		std::vector<size_t> mipBytesArr(initData ? mipLevels : 0);
		std::vector<Vec3uz> mipDimArr(initData ? mipLevels : 0);
		Image::calcMipsInfo(format, dim, mipLevels, &mipsBytes, initData ? mipBytesArr.data() : nullptr, initData ? mipDimArr.data() : nullptr);
		_size = mipsBytes * arraySize;

		if (initData) {
			if (_vkFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
				void* dst;
				if (_map(dst)) {
					size_t offset = 0;
					for (size_t i = 0; i < mipLevels; ++i) {
						memcpy((uint8_t*)dst + offset, data[i], mipBytesArr[i]);
						offset += mipBytesArr[i];
					}
					_unmap();
				} else {
					destroy();
					return false;
				}
				
			} else {
				BaseBuffer src;
				if (!src.create(graphics, _size, Usage::MAP_WRITE | Usage::COPY_SRC, Usage::NONE, data, _size)) {
					destroy();
					return false;
				}

				auto cmd = graphics.beginOneTimeCommands();
				if (!cmd) {
					destroy();
					return false;
				}

				VkImageMemoryBarrier imageMemoryBarrier;
				memset(&imageMemoryBarrier, 0, sizeof(imageMemoryBarrier));
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.oldLayout = imageCreateInfo.initialLayout;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.image = _image;
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
				imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
				imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
				imageMemoryBarrier.subresourceRange.layerCount = arraySize;

				vkCmdPipelineBarrier(cmd.getVkCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

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

				if (!graphics.endOneTimeCommands(cmd)) {
					destroy();
					return false;
				}
			}
		}

		_dim = dim;

		return true;
	}

	Usage BaseTexture::map(Usage expectMapUsage) {
		using namespace srk::enum_operators;

		auto ret = Usage::NONE;

		auto usage = expectMapUsage & _usage & Usage::MAP_READ_WRITE;

		if (usage == Usage::NONE) {
			unmap();
		} else {
			ret = usage;

			if (_mapUsage != usage) {
				unmap();

				if (usage != Usage::NONE) {
					if (_map(_mappedData)) _mapUsage = usage;
				} else {
					ret = Usage::NONE;
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

	void BaseTexture::unmap() {
		if (_mapUsage != Usage::NONE) {
			_mapUsage = Usage::NONE;

			_unmap();
			_mappedData = nullptr;
		}
	}

	void BaseTexture::_unmap() {
		if (!(_vkFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) vmaFlushAllocation(_memAllocator, _allocation, 0, _size);
		vmaUnmapMemory(_memAllocator, _allocation);
	}

	size_t BaseTexture::read(void* dst, size_t dstLen, size_t offset) {
		using namespace srk::enum_operators;

		if ((_mapUsage & Usage::MAP_READ) == Usage::MAP_READ) {
			if (!dstLen || offset >= _size) return 0;
			if (dst) {
				auto readLen = std::min<size_t>(_size - offset, dstLen);
				memcpy(dst, (uint8_t*)_mappedData + offset, readLen);
				return readLen;
			}
		}
		return -1;
	}

	size_t BaseTexture::write(const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((_mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
			if (data && length && offset < _size) {
				length = std::min<size_t>(length, _size - offset);
				memcpy((uint8_t*)_mappedData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	size_t BaseTexture::update(const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((_internalUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (data && length && offset < _size) {
				length = std::min<size_t>(length, _size - offset);

				if ((_mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
					memcpy((uint8_t*)_mappedData + offset, data, length);
				} else {
					auto old = _mapUsage;
					if (map(Usage::MAP_WRITE) != Usage::NONE) memcpy((uint8_t*)_mappedData + offset, data, length);
					map(old);
				}
				return length;
			}
			return 0;
		}
		return -1;
	}

	/*size_t BaseTexture::copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange) {
		if (dstPos >= _size || srcRange.pos[0] >= src._size) return 0;

		auto copySize = std::min(std::min(src._size - srcRange.pos[0], srcRange.size[0]), _size - dstPos);

		auto device = graphics.getDevice();
		auto commandPool = graphics.getCommandPool();

		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		memset(&commandBufferAllocateInfo, 0, sizeof(commandBufferAllocateInfo));
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS) return -1;

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		memset(&commandBufferBeginInfo, 0, sizeof(commandBufferBeginInfo));
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = srcRange.pos[0];
		bufferCopy.dstOffset = dstPos;
		bufferCopy.size = copySize;
		vkCmdCopyBuffer(commandBuffer, src._buffer, _buffer, 1, &bufferCopy);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo;
		memset(&submitInfo, 0, sizeof(submitInfo));
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		auto queue = graphics.getGraphicsQueue();

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

		return copySize;
	}

	size_t BaseTexture::copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		using namespace srk::enum_operators;

		if (!_buffer || (_usage & Usage::COPY_DST) != Usage::COPY_DST || !src || (src->getUsage() & Usage::COPY_SRC) != Usage::COPY_SRC || src->getGraphics() != graphics) return -1;

		auto srcNative = (const BaseBuffer*)src->getNative();
		if (!srcNative || !srcNative->_buffer) return -1;

		return copyFrom(graphics, dstPos, *srcNative, srcRange);
	}*/

	void BaseTexture::destroy() {
		unmap();

		if (_image) {
			vmaDestroyImage(_memAllocator, _image, _allocation);
			_image = nullptr;
		}

		_sampleCount = 0;
		_dim = 0;

		_memAllocator = nullptr;
		_allocation = nullptr;
		_size = 0;
		_usage = Usage::NONE;
		_internalUsage = Usage::NONE;
		_mapUsage = Usage::NONE;
		_mappedData = nullptr;
		_vkFlags = 0;
	}
}