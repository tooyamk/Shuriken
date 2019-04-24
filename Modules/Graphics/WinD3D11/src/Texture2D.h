#pragma once

#include "BaseResource.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Texture2D : public ITexture2D {
	public:
		Texture2D(Graphics& graphics);
		virtual ~Texture2D();

		virtual TextureType AE_CALL getType() const override;
		virtual bool AE_CALL allocate(ui32 width, ui32 height, TextureFormat format, ui32 mipLevels, ui32 bufferUsage, const void* data = nullptr) override;

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
		BaseResource _baseRes;
		ID3D11ShaderResourceView* _view;

		void AE_CALL _delTex();
		inline static ui32 AE_CALL _calcMipLevels(ui32 width, ui32 height) {
			return (ui32)std::floor(std::log2(std::max<ui32>(width, height)) + 1);
		}
	};
}