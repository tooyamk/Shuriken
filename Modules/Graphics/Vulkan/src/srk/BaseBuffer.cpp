#include "BaseBuffer.h"

namespace srk::modules::graphics::vulkan {
	BaseBuffer::BaseBuffer(VkBufferUsageFlags vkUsage) :
		_vkUsage(vkUsage),
		_vkFlags(0),
		_memAllocator(nullptr),
		_buffer(nullptr),
		_allocation(nullptr),
		_size(0),
		_usage(Usage::NONE),
		_internalUsage(Usage::NONE),
		_mapUsage(Usage::NONE),
		_mappedData(nullptr) {
	}

	BaseBuffer::~BaseBuffer() {
		destroy();
	}

	bool BaseBuffer::create(Graphics& graphics, size_t size, Usage usage, const void* data, size_t dataSize) {
		using namespace srk::enum_operators;

		destroy();

		usage &= Usage::CREATE_ALL;
		auto supportedUsages = graphics.getBufferCreateUsageMask();
		if ((usage & Usage::IGNORE_UNSUPPORTED) == Usage::IGNORE_UNSUPPORTED) {
			usage &= supportedUsages;
		} else if (auto u = (usage & (~supportedUsages)); u != Usage::NONE) {
			graphics.error(std::format("Vulkan BaseBuffer::create error : has not support Usage {}", (std::underlying_type_t<Usage>)u));
			return false;
		}

		VkBufferCreateInfo bufferCreateInfo;
		memset(&bufferCreateInfo, 0, sizeof(bufferCreateInfo));
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = _vkUsage;
		if ((usage & Usage::COPY_SRC) == Usage::COPY_SRC) bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if ((usage & Usage::COPY_DST) == Usage::COPY_DST) bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto initData = data && dataSize;

		auto cpuRead = (usage & Usage::MAP_READ) == Usage::MAP_READ;
		auto cpuWrite = (usage & Usage::MAP_WRITE_UPDATE) != Usage::NONE;

		_internalUsage = Usage::NONE;
		if (initData && !cpuWrite) {
			_internalUsage |= Usage::COPY_DST;
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		VmaMemoryUsage vmaUsage;
		if (cpuRead) {
			vmaUsage = cpuWrite ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_TO_CPU;
		} else if (cpuWrite) {
			vmaUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		} else {
			vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		}

		VmaAllocationCreateInfo allocationCreateInfo;
		memset(&allocationCreateInfo, 0, sizeof(allocationCreateInfo));
		allocationCreateInfo.usage = vmaUsage;

		_memAllocator = graphics.getMemAllocator();
		_size = size;
		_usage = usage;
		_internalUsage |= _usage;
		if ((_internalUsage & Usage::UPDATE) == Usage::UPDATE) _internalUsage |= Usage::MAP_WRITE;

		auto z = vkCreateBuffer(graphics.getDevice(), &bufferCreateInfo, nullptr, &_buffer);

		/*VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(graphics.getDevice(), _buffer, &memRequirements);*/

		/*VkBufferMemoryRequirementsInfo2 memReqInfo = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2 };
		memReqInfo.buffer = _buffer;
		VkMemoryRequirements2 memReq2;*/
		//vkGetDeviceProcAddr(graphics.getDevice());
		//auto vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2)vkGetBufferMemoryRequirements2;
		/*vkGetBufferMemoryRequirements2(graphics.getDevice(), &memReqInfo, &memReq2);

		vkDestroyBuffer(graphics.getDevice(), _buffer, nullptr);*/

		if (vmaCreateBuffer(_memAllocator, &bufferCreateInfo, &allocationCreateInfo, &_buffer, &_allocation, nullptr) != VK_SUCCESS) {
			destroy();
			return false;
		}

		vmaGetAllocationMemoryProperties(_memAllocator, _allocation, &_vkFlags);

		if (initData) {
			auto n = std::min(_size, dataSize);
			if (_vkFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
				void* dst;
				if (_map(dst)) {
					memcpy(dst, data, n);
					_unmap();
				} else {
					destroy();
					return false;
				}
			} else {
				BaseBuffer src(_vkUsage);
				if (!src.create(graphics, n, Usage::MAP_WRITE | Usage::COPY_SRC, data, n)) {
					destroy();
					return false;
				}
				if (copyFrom(graphics, 0, src, Box1uz(Vec1uz(0), Vec1uz(n))) == -1) {
					destroy();
					return false;
				}
			}
		}

		return true;
	}

	Usage BaseBuffer::map(Usage expectMapUsage) {
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

	bool BaseBuffer::_map(void*& mappedData) {
		if (vmaMapMemory(_memAllocator, _allocation, &mappedData) != VK_SUCCESS) return false;
		if (!(_vkFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) vmaInvalidateAllocation(_memAllocator, _allocation, 0, _size);
		return true;
	}

	void BaseBuffer::unmap() {
		if (_mapUsage != Usage::NONE) {
			_mapUsage = Usage::NONE;

			_unmap();
			_mappedData = nullptr;
		}
	}

	void BaseBuffer::_unmap() {
		if (!(_vkFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) vmaFlushAllocation(_memAllocator, _allocation, 0, _size);
		vmaUnmapMemory(_memAllocator, _allocation);
	}

	size_t BaseBuffer::read(void* dst, size_t dstLen, size_t offset) {
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

	size_t BaseBuffer::write(const void* data, size_t length, size_t offset) {
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

	size_t BaseBuffer::update(const void* data, size_t length, size_t offset) {
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

	size_t BaseBuffer::copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange) {
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

	size_t BaseBuffer::copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange) {
		using namespace srk::enum_operators;

		if (!_buffer || (_usage & Usage::COPY_DST) != Usage::COPY_DST || !src || (src->getUsage() & Usage::COPY_SRC) != Usage::COPY_SRC || src->getGraphics() != graphics) return -1;

		auto srcNative = (const BaseBuffer*)src->getNative();
		if (!srcNative || !srcNative->_buffer) return -1;

		return copyFrom(graphics, dstPos, *srcNative, srcRange);
	}

	void BaseBuffer::destroy() {
		unmap();

		if (_buffer) {
			vmaDestroyBuffer(_memAllocator, _buffer, _allocation);
			_buffer = nullptr;
		}

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