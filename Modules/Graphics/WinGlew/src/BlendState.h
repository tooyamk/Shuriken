#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL BlendState : public IBlendState {
	public:
		static const uint8_t NUM_RTS = 8;

		struct Desc {
			bool independentBlendEnabled = false;
			RenderTargetBlendState renderTarget[NUM_RTS];
		};


		BlendState(Graphics& graphics);
		virtual ~BlendState();

		virtual bool AE_CALL isIndependentBlendEnabled() const override;
		virtual void AE_CALL setIndependentBlendEnabled(bool enalbed) override;

		virtual const RenderTargetBlendState& AE_CALL getRenderTargetState(uint8_t index) const override;
		virtual void AE_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) override;

		inline const Desc& AE_CALL getDesc() const {
			return _desc;
		}

		void AE_CALL update();

	protected:
		static const RenderTargetBlendState DEFAULT_RT_STATE;

		Desc _desc;
	};
}