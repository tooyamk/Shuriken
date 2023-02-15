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

		Usage SRK_CALL map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage);
		void SRK_CALL unmap(size_t arraySlice, size_t mipSlice);
		size_t SRK_CALL read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen);
		size_t SRK_CALL write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length);
		bool SRK_CALL update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data);
		bool SRK_CALL copyFrom(Graphics& graphics, const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange);

		void SRK_CALL destroy();

		inline static constexpr size_t SRK_CALL calcSubresource(size_t mipSlice, size_t arraySlice, size_t mipLevels) {
			return mipSlice + arraySlice * mipLevels;
		}

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
		VkImageLayout _imageLayout;
		VmaAllocation _allocation;

		Usage _usage;
		Usage _internalUsage;
		size_t _mapCount;
		size_t _size;
		size_t _memAlignment;
		size_t _mipLevels;
		size_t _arraySize;

		void* _mappedData;

		struct MapData {
			Usage usage;
			bool completelyAlign;
			size_t size;
			size_t sliceAlignSize;
			size_t begin;
			size_t rowBytes;
			size_t rowAlignSize;
		};

		std::vector<MapData> _mapData;

		bool _map(void*& mappedData);
		void _unmap();
		inline void _unmap(size_t subresource) {
			_unmap(_mapData[subresource]);
		}
		void _unmap(MapData& md);
		void _write(const MapData& md, const Box3uz& range, const void* data);
		void _tryRestoreImageLayout(VkCommandBuffer cmd, VkImageLayout cur, const VkImageSubresourceRange& range);
	};
}