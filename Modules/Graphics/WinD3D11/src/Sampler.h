#pragma once

#include "BaseResource.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Sampler : public ISampler {
	public:
		Sampler(Graphics& graphics);
		virtual ~Sampler();

		virtual void AE_CALL setFilter(SamplerFilterOperation op, SamplerFilterMode min, SamplerFilterMode mag, SamplerFilterMode mipmap) override;
		virtual void AE_CALL setFilter(const SamplerFilter& filter) override;

		template<ProgramStage stage>
		inline void AE_CALL use(UINT slot) {
		}

		template<>
		inline void AE_CALL use<ProgramStage::VS>(UINT slot) {
			_update();
			((Graphics*)_graphics)->getContext()->VSSetSamplers(slot, 1, &_samplerState);
		}

		template<>
		inline void AE_CALL use<ProgramStage::PS>(UINT slot) {
			_update();
			((Graphics*)_graphics)->getContext()->PSSetSamplers(slot, 1, &_samplerState);
		}

	protected:
		bool _dirty;
		SamplerFilter _filter;
		D3D11_SAMPLER_DESC _desc;
		ID3D11SamplerState* _samplerState;

		void _update();
		void _releaseRes();
	};
}