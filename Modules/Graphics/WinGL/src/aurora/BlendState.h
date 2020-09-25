#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_gl {
	class Graphics;

	class AE_MODULE_DLL BlendState : public IBlendState {
	public:
		BlendState(Graphics& graphics, bool isInternal);
		virtual ~BlendState();

		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL isIndependentBlendEnabled() const override;
		virtual void AE_CALL setIndependentBlendEnabled(bool enalbed) override;

		virtual const RenderTargetBlendState* AE_CALL getRenderTargetState(uint8_t index) const override;
		virtual bool AE_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) override;

		inline const InternalRenderTargetBlendState& AE_CALL getInternalRenderTargetState(uint8_t index) const {
			return _states[index];
		}

	protected:
		bool _isInternal;
		bool _independentBlendEnabled = false;
		const uint8_t MAX_MRT_COUNT;
		std::vector<InternalRenderTargetBlendState> _states;

		static uint16_t AE_CALL _convertBlendFactor(BlendFactor factor);
		static uint16_t AE_CALL _convertBlendOp(BlendOp op);

		void AE_CALL _updateInternalState(uint8_t index);
	};
}