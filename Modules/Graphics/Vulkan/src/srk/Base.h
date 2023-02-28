#pragma once

#include "srk/modules/graphics/GraphicsModule.h"
#include "vulkan/vulkan.h"

#if SRK_OS == SRK_OS_WINDOWS
#	include "vulkan/vulkan_win32.h"
#elif SRK_OS == SRK_OS_LINUX
#	include <X11/Xutil.h>
#	include "vulkan/vulkan_xlib.h"
#endif

#include "vk_mem_alloc.h"

namespace srk::modules::graphics::vulkan {
	class Graphics;

	class InternalCommandBuffer {
	public:
		InternalCommandBuffer() :
			_buffer(nullptr),
			_device(nullptr),
			_pool(nullptr) {
		}

		InternalCommandBuffer(const InternalCommandBuffer& buffer) = delete;

		InternalCommandBuffer(InternalCommandBuffer&& buffer) noexcept :
			_buffer(buffer._buffer),
			_device(buffer._device),
			_pool(buffer._pool) {
			buffer._buffer = nullptr;
		}

		~InternalCommandBuffer() {
			release();
		}

		inline SRK_CALL operator bool() const {
			return _buffer;
		}

		inline InternalCommandBuffer& SRK_CALL operator=(InternalCommandBuffer&& value) noexcept {
			_buffer = value._buffer;
			_device = value._device;
			_pool = value._pool;

			value._buffer = nullptr;

			return *this;
		}

		bool create(VkDevice device, VkCommandPool pool, VkCommandBufferAllocateInfo& info) {
			release();

			if (vkAllocateCommandBuffers(device, &info, &_buffer) != VK_SUCCESS) return false;

			_device = device;
			_pool = pool;

			return true;
		}

		void release() {
			if (_buffer) {
				vkFreeCommandBuffers(_device, _pool, 1, &_buffer);
				_buffer = nullptr;
			}
		}

		inline VkCommandBuffer SRK_CALL getVkCommandBuffer() const {
			return _buffer;
		}

	private:
		VkCommandBuffer _buffer;
		VkDevice _device;
		VkCommandPool _pool;
	};
}