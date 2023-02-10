#pragma once

#include "BaseResource.h"
#include <unordered_set>

namespace srk::modules::graphics::d3d11 {
	class BaseTextureResource : public BaseResource {
	public:
		BaseTextureResource(D3D11_BIND_FLAG resType);
		virtual ~BaseTextureResource();

		bool SRK_CALL create(Graphics& graphics, TextureType texType, const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount,
			TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data = nullptr);
		Usage SRK_CALL map(Graphics& graphics, size_t arraySlice, size_t mipSlice, Usage expectMapUsage);
		void SRK_CALL unmap(Graphics& graphics, size_t arraySlice, size_t mipSlice);
		size_t SRK_CALL read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen);
		size_t SRK_CALL write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length);
		bool SRK_CALL update(Graphics& graphics, size_t arraySlice, size_t mipSlice, const D3D11_BOX& range, const void* data);
		bool SRK_CALL copyFrom(Graphics& graphics, const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange);
		void SRK_CALL releaseTex(Graphics& graphics);

		inline static constexpr UINT SRK_CALL calcSubresource(UINT mipSlice, UINT arraySlice, UINT mipLevels) {
			return mipSlice + arraySlice * mipLevels;
		}

		TextureFormat format;
		DXGI_FORMAT internalFormat;
		SampleCount sampleCount;
		size_t perBlockBytes;
		size_t perRowPixels;
		Vec3uz dim;
		size_t arraySize;
		size_t internalArraySize;
		size_t mipLevels;

		struct MapData {
			Usage usage;
			size_t size;
			D3D11_MAPPED_SUBRESOURCE res;
		};

		union TexDesc {
			D3D11_TEXTURE1D_DESC dsec1D;
			D3D11_TEXTURE2D_DESC dsec2D;
			D3D11_TEXTURE3D_DESC dsec3D;
		};

		std::vector<MapData> mapData;

	private:
		HRESULT SRK_CALL _createInternalTexture(Graphics& graphics, TextureType texType, const TexDesc& desc, const D3D11_SUBRESOURCE_DATA* pInitialData);
		bool SRK_CALL _createDone(Graphics& graphics, bool succeeded);
	};
}