#include "BaseBuffer.h"
#include <format>

namespace srk::modules::graphics::vulkan {
	BaseBuffer::BaseBuffer() : BaseBuffer((VkBufferUsageFlags)0) {
	}

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

	bool BaseBuffer::create(Graphics& graphics, size_t size, Usage requiredUsage, Usage preferredUsage, const void* data, size_t dataSize) {
		using namespace srk::enum_operators;

		destroy();

		requiredUsage &= Usage::BUFFER_CREATE_ALL;
		preferredUsage &= Usage::BUFFER_CREATE_ALL;
		auto supportedUsages = graphics.getBufferCreateUsageMask();
		if (auto u = (requiredUsage & (~graphics.getBufferCreateUsageMask())); u != Usage::NONE) {
			graphics.error(std::format("Vulkan BaseBuffer::create error : has not support Usage {}", (std::underlying_type_t<Usage>)u));
			return false;
		}

		auto allUsage = requiredUsage | preferredUsage;
		_usage = Usage::NONE;

		VkBufferCreateInfo bufferCreateInfo;
		memset(&bufferCreateInfo, 0, sizeof(bufferCreateInfo));
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = _vkUsage;
		if ((allUsage & Usage::COPY_SRC) == Usage::COPY_SRC) {
			_usage |= Usage::COPY_SRC;
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if ((allUsage & Usage::COPY_DST) == Usage::COPY_DST) {
			_usage |= Usage::COPY_DST;
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto initData = data && dataSize;

		auto cpuRead = (allUsage & Usage::MAP_READ) == Usage::MAP_READ;
		auto cpuWriteUsage = allUsage & Usage::MAP_WRITE_UPDATE;
		auto cpuWrite = cpuWriteUsage != Usage::NONE;

		_internalUsage = Usage::NONE;
		if (initData && !cpuWrite) {
			_internalUsage |= Usage::COPY_DST;
			bufferCreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
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
			graphics.error(std::format("Vulkan BaseBuffer::create error : has not support preferredUsage {}", (std::underlying_type_t<Usage>)(requiredUsage & (~(_usage & requiredUsage)))));
			destroy();
			return false;
		}

		VmaAllocationCreateInfo allocationCreateInfo;
		memset(&allocationCreateInfo, 0, sizeof(allocationCreateInfo));
		allocationCreateInfo.usage = vmaUsage;

		_memAllocator = graphics.getMemAllocator();
		_size = size;
		_internalUsage |= _usage;
		if ((_internalUsage & Usage::UPDATE) == Usage::UPDATE) _internalUsage |= Usage::MAP_WRITE;

		/*if (vkCreateBuffer(graphics.getDevice(), &bufferCreateInfo, graphics.getVkAllocationCallbacks(), &_buffer) != VK_SUCCESS) {
			destroy();
			return false;
		}
		
		VkMemoryRequirements memRequirements;
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
				BaseBuffer src;
				if (!src.create(graphics, n, Usage::MAP_WRITE | Usage::COPY_SRC, Usage::NONE, data, n)) {
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

		auto usage = expectMapUsage & _internalUsage & Usage::MAP_READ_WRITE;

		if (usage == Usage::NONE) {
			unmap();
		} else {
			ret = usage;

			if (_mapUsage != usage) {
				if (_mappedData) {
					_mapUsage = usage;
				} else {
					if (_map(_mappedData)) {
						_mapUsage = usage;
					} else {
						unmap();
						ret = Usage::NONE;
					}
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
			if (!dst || !dstLen || offset >= _size) return 0;
			
			auto readLen = std::min<size_t>(_size - offset, dstLen);
			memcpy(dst, (uint8_t*)_mappedData + offset, readLen);
			return readLen;
		}
		return -1;
	}

	size_t BaseBuffer::write(const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((_mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
			if (!data || !length || offset >= _size) return 0;
			
			length = std::min<size_t>(length, _size - offset);
			memcpy((uint8_t*)_mappedData + offset, data, length);
			return length;
		}
		return -1;
	}

	size_t BaseBuffer::update(const void* data, size_t length, size_t offset) {
		using namespace srk::enum_operators;

		if ((_internalUsage & Usage::UPDATE) == Usage::UPDATE) {
			if (!data && !length && offset >= _size) return 0;

			length = std::min<size_t>(length, _size - offset);
			if ((_mapUsage & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
				memcpy((uint8_t*)_mappedData + offset, data, length);
			} else {
				auto old = _mapUsage;
				if (map(Usage::MAP_WRITE) == Usage::NONE) {
					length = -1;
				} else {
					memcpy((uint8_t*)_mappedData + offset, data, length);
				}
				map(old);
			}
			return length;
		}
		return -1;
	}

	size_t BaseBuffer::copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange) {
		if (dstPos >= _size || srcRange.pos[0] >= src._size) return 0;

		auto copySize = std::min(std::min(src._size - srcRange.pos[0], srcRange.size[0]), _size - dstPos);

		auto cmd = graphics.beginOneTimeCommands();
		if (!cmd) return -1;

		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = srcRange.pos[0];
		bufferCopy.dstOffset = dstPos;
		bufferCopy.size = copySize;
		vkCmdCopyBuffer(cmd.getVkCommandBuffer(), src._buffer, _buffer, 1, &bufferCopy);

		if (!graphics.endOneTimeCommands(cmd)) return -1;

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