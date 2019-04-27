#pragma once

#include "BaseResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	class BaseTexture : public BaseResource {
	public:
		BaseTexture(UINT resType);
		virtual ~BaseTexture();

		bool AE_CALL allocate(Graphics* graphics, TextureType texType, ui32 width, ui32 height, ui32 depth, i32 arraySize, 
			TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data = nullptr);
		Usage AE_CALL map(Graphics* graphics, ui32 mipLevel, Usage expectMapUsage);
		void AE_CALL unmap(Graphics* graphics, ui32 mipLevel);
		i32 AE_CALL read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1);
		i32 AE_CALL write(ui32 mipLevel, ui32 offset, const void* data, ui32 length);
		bool AE_CALL write(Graphics* graphics, ui32 mipLevel, const D3D11_BOX& range, const void* data);
		void AE_CALL releaseTex(Graphics* graphics);

		TextureFormat format;
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

		ID3D11ShaderResourceView* view;

		HRESULT create(Graphics* graphics, TextureType texType, const TexDesc& desc, const D3D11_SUBRESOURCE_DATA *pInitialData);

		template<ProgramStage stage>
		inline void AE_CALL use(Graphics* graphics, UINT slot) {
		}

		template<>
		inline void AE_CALL use<ProgramStage::VS>(Graphics* graphics, UINT slot) {
			graphics->getContext()->VSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&view);
		}

		template<>
		inline void AE_CALL use<ProgramStage::PS>(Graphics* graphics, UINT slot) {
			graphics->getContext()->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&view);
		}
	};
}