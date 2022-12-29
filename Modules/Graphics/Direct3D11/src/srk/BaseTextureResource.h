#pragma once

#include "BaseResource.h"
#include <unordered_set>

namespace srk::modules::graphics::d3d11 {
	class BaseTextureResource : public BaseResource {
	public:
		BaseTextureResource(D3D11_BIND_FLAG resType);
		virtual ~BaseTextureResource();

		bool SRK_CALL create(Graphics& graphics, TextureType texType, const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, SampleCount sampleCount,
			TextureFormat format, Usage resUsage, const void*const* data = nullptr);
		Usage SRK_CALL map(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage);
		void SRK_CALL unmap(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice);
		uint32_t SRK_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen);
		uint32_t SRK_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length);
		bool SRK_CALL update(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, const D3D11_BOX& range, const void* data);
		bool SRK_CALL copyFrom(Graphics& graphics, const Vec3ui32& dstPos, uint32_t dstArraySlice, uint32_t dstMipSlice, const ITextureResource* src, uint32_t srcArraySlice, uint32_t srcMipSlice, const Box3ui32& srcRange);
		void SRK_CALL releaseTex(Graphics& graphics);

		inline static constexpr UINT SRK_CALL calcSubresource(UINT mipSlice, UINT arraySlice, UINT mipLevels) {
			return mipSlice + arraySlice * mipLevels;
		}

		TextureFormat format;
		DXGI_FORMAT internalFormat;
		SampleCount sampleCount;
		uint16_t perPixelSize;
		uint32_t perRowPixelSize;
		Vec3ui32 texSize;
		uint32_t arraySize;
		uint32_t internalArraySize;
		uint32_t mipLevels;

		struct MappedRes {
			Usage usage;
			uint32_t size;
			D3D11_MAPPED_SUBRESOURCE res;
		};

		union TexDesc {
			D3D11_TEXTURE1D_DESC dsec1D;
			D3D11_TEXTURE2D_DESC dsec2D;
			D3D11_TEXTURE3D_DESC dsec3D;
		};

		std::vector<MappedRes> mappedRes;

	private:
		HRESULT SRK_CALL _createInternalTexture(Graphics& graphics, TextureType texType, const TexDesc& desc, const D3D11_SUBRESOURCE_DATA* pInitialData);
		bool SRK_CALL _createDone(Graphics& graphics, bool succeeded);
	};
}