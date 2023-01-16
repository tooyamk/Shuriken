#pragma once

#include "Base.h"
#include "Graphics.h"

namespace srk::modules::graphics::vulkan {
	class BaseTexture {
	public:
		BaseTexture(TextureType texType);
		~BaseTexture();

		bool SRK_CALL create(Graphics& graphics, const Vec3uz& size, size_t arraySize, size_t mipLevels, SampleCount sampleCount,
			TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data = nullptr);

		Usage SRK_CALL map(Usage expectMapUsage);
		void SRK_CALL unmap();
		size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset);
		size_t SRK_CALL write(const void* data, size_t length, size_t offset);
		size_t SRK_CALL update(const void* data, size_t length, size_t offset);

		//size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const BaseBuffer& src, const Box1uz& srcRange);
		//size_t SRK_CALL copyFrom(Graphics& graphics, size_t dstPos, const IBuffer* src, const Box1uz& srcRange);

		void SRK_CALL destroy();

		inline TextureType SRK_CALL getTexType() const {
			return _texType;
		}

		inline SampleCount SRK_CALL getSampleCount() const {
			return _sampleCount;
		}

		inline TextureFormat SRK_CALL getFormat() const {
			return _format;
		}

		inline const Vec3uz& SRK_CALL getDimensions() const {
			return _dim;
		}

		inline Usage SRK_CALL getUsage() const {
			return _usage;
		}

		inline size_t SRK_CALL getSize() const {
			return _size;
		}

		inline VkImage SRK_CALL getVkImage() const {
			return _image;
		}

	private:
		SampleCount _sampleCount;
		TextureType _texType;
		TextureFormat _format;
		//Usage mapUsage;
		Vec3uz _dim;

		VkMemoryPropertyFlags _vkFlags;

		VmaAllocator _memAllocator;
		VkImage _image;
		VmaAllocation _allocation;

		Usage _usage;
		Usage _internalUsage;
		Usage _mapUsage;
		size_t _size;

		void* _mappedData;
		size_t _mappedDataOffset;
		size_t _mappedDataSize;

		bool _map(void*& mappedData);
		void _unmap();
	};
}