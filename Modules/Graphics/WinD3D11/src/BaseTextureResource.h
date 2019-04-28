#pragma once

#include "BaseResource.h"

namespace aurora::modules::graphics::win_d3d11 {
	class BaseTextureResource : public BaseResource {
	public:
		BaseTextureResource(UINT resType);
		virtual ~BaseTextureResource();

		bool AE_CALL create(Graphics* graphics, TextureType texType, ui32 width, ui32 height, ui32 depth, i32 arraySize, 
			TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data = nullptr);
		Usage AE_CALL map(Graphics* graphics, ui32 mipLevel, Usage expectMapUsage);
		void AE_CALL unmap(Graphics* graphics, ui32 mipLevel);
		i32 AE_CALL read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1);
		i32 AE_CALL write(ui32 mipLevel, ui32 offset, const void* data, ui32 length);
		bool AE_CALL write(Graphics* graphics, ui32 mipLevel, const D3D11_BOX& range, const void* data);
		void AE_CALL releaseTex(Graphics* graphics);
		void AE_CALL addView(ITextureView& view, const std::function<void()>& onRecreated);
		void AE_CALL removeView(ITextureView& view);

		TextureFormat format;
		DXGI_FORMAT internalFormat;
		ui16 perPixelSize;
		ui32 width;
		ui32 height;
		ui32 depth;
		ui32 arraySize;
		ui32 mipLevels;

		struct MappedRes {
			Usage usage;
			ui32 size;
			D3D11_MAPPED_SUBRESOURCE res;
		};

		union TexDesc {
			D3D11_TEXTURE1D_DESC dsec1D;
			D3D11_TEXTURE2D_DESC dsec2D;
			D3D11_TEXTURE3D_DESC dsec3D;
		};

		std::vector<MappedRes> mappedRes;

		std::unordered_map<ITextureView*, std::function<void()>> views;

	private:
		HRESULT AE_CALL _createInternalTexture(Graphics* graphics, TextureType texType, const TexDesc& desc, const D3D11_SUBRESOURCE_DATA* pInitialData);
		bool AE_CALL _createDone(bool succeeded);
	};
}