#pragma once

#include "Base.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	class BaseBuffer {
	public:
		BaseBuffer(VkBufferUsageFlags vkUsage);
		~BaseBuffer();

		bool SRK_CALL create(Graphics& graphics, size_t size, Usage usage, const void* data, size_t dataSize);

		Usage SRK_CALL map(Usage expectMapUsage);
		void SRK_CALL unmap();
		size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset);
		size_t SRK_CALL write(const void* data, size_t length, size_t offset);
		size_t SRK_CALL update(const void* data, size_t length, size_t offset);

		size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange);
		size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange);

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
		VkMemoryPropertyFlags _vkFlags;

		VmaAllocator _memAllocator;
		VkBuffer _buffer;
		VmaAllocation _allocation;

		Usage _usage;
		Usage _internalUsage;
		Usage _mapUsage;
		size_t _size;

		void* _mappedData;

		bool _map(void*& mappedData);
		void _unmap();
	};
}