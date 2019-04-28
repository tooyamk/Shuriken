#pragma once

#include "BaseResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Sampler : public ISampler {
	public:
		Sampler(Graphics& graphics);
		virtual ~Sampler();

		virtual void AE_CALL setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) override;
		virtual void AE_CALL setComparisonFunc(SamplerComparisonFunc func) override;
		virtual void AE_CALL setAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) override;
		virtual void AE_CALL setMipLOD(f32 min, f32 max, f32 bias) override;
		virtual void AE_CALL setMaxAnisotropy(ui32 max) override;
		virtual void AE_CALL setBorderColor(const Vector4& color) override;

		template<ProgramStage stage>
		inline void AE_CALL use(UINT slot) {
		}

		template<>
		inline void AE_CALL use<ProgramStage::VS>(UINT slot) {
			_update();
			_graphics.get<Graphics>()->getContext()->VSSetSamplers(slot, 1, &_samplerState);
		}

		template<>
		inline void AE_CALL use<ProgramStage::PS>(UINT slot) {
			_update();
			_graphics.get<Graphics>()->getContext()->PSSetSamplers(slot, 1, &_samplerState);
		}

	protected:
		bool _dirty;
		SamplerFilter _filter;
		SamplerAddress _address;
		D3D11_SAMPLER_DESC _desc;
		ID3D11SamplerState* _samplerState;

		void _updateFilter();

		static D3D11_COMPARISON_FUNC _convertComparisonFunc(SamplerComparisonFunc func);
		static D3D11_TEXTURE_ADDRESS_MODE _convertAddressMode(SamplerAddressMode mode);
		void _updateAddress();

		void _update();

		void _releaseRes();
	};
}