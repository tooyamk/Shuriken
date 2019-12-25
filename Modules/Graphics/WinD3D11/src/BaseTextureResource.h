#pragma once

#include "BaseResource.h"
#include <unordered_set>

namespace aurora::modules::graphics::win_d3d11 {
	class TextureView;


	class BaseTextureResource : public BaseResource {
	public:
		BaseTextureResource(UINT resType);
		virtual ~BaseTextureResource();

		bool AE_CALL create(Graphics& graphics, TextureType texType, const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels,
			TextureFormat format, Usage resUsage, const void*const* data = nullptr);
		Usage AE_CALL map(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage);
		void AE_CALL unmap(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice);
		uint32_t AE_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen);
		uint32_t AE_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length);
		bool AE_CALL update(Graphics& graphics, uint32_t arraySlice, uint32_t mipSlice, const D3D11_BOX& range, const void* data);
		void AE_CALL releaseTex(Graphics& graphics);
		void AE_CALL addView(TextureView& view);
		void AE_CALL removeView(TextureView& view);

		inline static constexpr UINT calcSubresource(UINT mipSlice, UINT arraySlice, UINT mipLevels) {
			return mipSlice + arraySlice * mipLevels;
		}

		TextureFormat format;
		DXGI_FORMAT internalFormat;
		uint16_t perPixelSize;
		Vec3ui32 texSize;
		uint32_t arraySize;
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

		std::unordered_set<TextureView*> views;

	private:
		HRESULT AE_CALL _createInternalTexture(Graphics& graphics, TextureType texType, const TexDesc& desc, const D3D11_SUBRESOURCE_DATA* pInitialData);
		bool AE_CALL _createDone(bool succeeded);
	};
}