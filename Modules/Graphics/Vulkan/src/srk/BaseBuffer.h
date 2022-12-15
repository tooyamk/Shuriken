#pragma once

#include "Base.h"

namespace srk::modules::graphics::vulkan {
	class BaseBuffer {
	public:
		BaseBuffer(VkBufferUsageFlags vkUsage);

		bool SRK_CALL create(Graphics& graphics, size_t size, Usage usage, const void* data, size_t dataSize);

		Usage SRK_CALL map(Usage expectMapUsage);
		void SRK_CALL unmap();
		size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset);
		size_t SRK_CALL write(const void* data, size_t length, size_t offset);
		size_t SRK_CALL update(const void* data, size_t length, size_t offset);

		void SRK_CALL destroy();

		inline Usage SRK_CALL getUsage() const {
			return _usage;
		}

		inline size_t SRK_CALL getSize() const {
			return _size;
		}

		inline VkBuffer SRK_CALL getBuffer() const {
			return _buffer;
		}

	private:
		VkBufferUsageFlags _vkUsage;

		VkDevice _device;
		VkBuffer _buffer;
		VkDeviceMemory _mem;

		Usage _usage;
		Usage _internalUsage;
		Usage _mapUsage;
		size_t _size;

		void* _mapData;
	};
}