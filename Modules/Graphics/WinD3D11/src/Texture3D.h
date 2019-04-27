#pragma once

#include "BaseResource.h"
#include "Graphics.h"

namespace aurora {
	template<typename T> class Box;
}

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Texture3D : public ITexture2D {
	public:
		Texture3D(Graphics& graphics);
		virtual ~Texture3D();

		virtual TextureType AE_CALL getType() const override;
		virtual bool AE_CALL allocate(ui32 width, ui32 height, ui32 depth, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data = nullptr) ;
		virtual Usage AE_CALL map(ui32 mipLevel, Usage mapUsage) override;
		virtual void AE_CALL unmap(ui32 mipLevel) override;
		virtual i32 AE_CALL read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) override;
		virtual bool AE_CALL write(ui32 mipLevel, const Box<ui32>& range, const void* data) ;

		template<ProgramStage stage>
		inline void AE_CALL use(UINT slot) {
		}

		template<>
		inline void AE_CALL use<ProgramStage::VS>(UINT slot) {
			((Graphics*)_graphics)->getContext()->VSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&_view);
		}

		template<>
		inline void AE_CALL use<ProgramStage::PS>(UINT slot) {
			((Graphics*)_graphics)->getContext()->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&_view);
		}

	protected:
		TextureFormat _format;
		ui16 _perPixelSize;
		ui32 _width;
		ui32 _height;
		ui32 _depth;
		ui32 _mipLevels;

		struct MappedRes {
			Usage usage;
			ui32 size;
			D3D11_MAPPED_SUBRESOURCE res;
		};

		std::vector<MappedRes> _mappedRes;

		BaseResource _baseRes;
		ID3D11ShaderResourceView* _view;

		void AE_CALL _releaseTex();
	};
}