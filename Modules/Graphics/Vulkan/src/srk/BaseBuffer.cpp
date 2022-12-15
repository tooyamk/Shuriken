#include "BaseBuffer.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	BaseBuffer::BaseBuffer(VkBufferUsageFlags vkUsage) :
		_vkUsage(vkUsage),
		_device(nullptr),
		_buffer(nullptr),
		_mem(nullptr),
		_size(0),
		_usage(Usage::NONE),
		_internalUsage(Usage::NONE),
		_mapUsage(Usage::NONE) {
	}

	bool BaseBuffer::create(Graphics& graphics, size_t size, Usage usage, const void* data, size_t dataSize) {
		using namespace srk::enum_operators;

		destroy();

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
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		_device = graphics.getDevice();
		_size = size;
		_usage = usage;
		_internalUsage = _usage;
		if ((_internalUsage & Usage::UPDATE) == Usage::UPDATE) _internalUsage |= Usage::MAP_WRITE;

		if (vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &_buffer) != VK_SUCCESS) {
			destroy();
			return false;
		}

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(_device, _buffer, &memoryRequirements);
		auto index = graphics.findProperties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (index < 0) {
			destroy();
			return false;
		}

		VkMemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = index;
		if (vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &_mem) != VK_SUCCESS) {
			destroy();
			return false;
		}

		if (vkBindBufferMemory(_device, _buffer, _mem, 0) != VK_SUCCESS) {
			destroy();
			return false;
		}

		if (data && dataSize) {
			auto n = std::min(_size, dataSize);
			void* dst;
			if (vkMapMemory(_device, _mem, 0, n, 0, &dst) == VK_SUCCESS) {
				memcpy(dst, data, n);
				vkUnmapMemory(_device, _mem);
			} else {
				destroy();
				return false;
			}
		}

		return true;
	}

	Usage BaseBuffer::map(Usage expectMapUsage) {
		using namespace srk::enum_operators;

		auto ret = Usage::NONE;

		if (_mem) {
			auto usage = expectMapUsage & _usage & Usage::MAP_READ_WRITE;

			if (usage == Usage::NONE) {
				unmap();
			} else {
				ret = usage;

				if (_mapUsage != usage) {
					unmap();

					if (usage != Usage::NONE && vkMapMemory(_device, _mem, 0, _size, 0, &_mapData) == VK_SUCCESS) {
						_mapUsage = usage;
					} else {
						ret = Usage::NONE;
					}
				}
			}
		}
		
		return ret;
	}

	void BaseBuffer::unmap() {
		if (_mapUsage != Usage::NONE) {
			_mapUsage = Usage::NONE;

			vkUnmapMemory(_device, _mem);
			_mapData = nullptr;
		}
	}

	size_t BaseBuffer::read(void* dst, size_t dstLen, size_t offset) {
		using namespace srk::enum_operators;

		if ((_mapUsage & Usage::MAP_READ) == Usage::MAP_READ) {
			if (!dstLen || offset >= _size) return 0;
			if (dst) {
				auto readLen = std::min<size_t>(_size - offset, dstLen);
				memcpy(dst, (uint8_t*)_mapData + offset, readLen);
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
				memcpy((uint8_t*)_mapData + offset, data, length);
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
					memcpy((uint8_t*)_mapData + offset, data, length);
				} else {
					auto old = _mapUsage;
					if (map(Usage::MAP_WRITE) != Usage::NONE) memcpy((uint8_t*)_mapData + offset, data, length);
					map(old);
				}
				return length;
			}
			return 0;
		}
		return -1;
	}

	void BaseBuffer::destroy() {
		if (_mem) {
			vkFreeMemory(_device, _mem, nullptr);
			_mem = nullptr;
		}

		if (_buffer) {
			vkDestroyBuffer(_device, _buffer, nullptr);
			_buffer = nullptr;
		}

		_device = nullptr;
		_size = 0;
		_usage = Usage::NONE;
		_internalUsage = Usage::NONE;
		_mapUsage = Usage::NONE;
		_mapData = nullptr;
	}
}