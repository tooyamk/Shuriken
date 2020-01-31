#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL BlendState : public IBlendState {
	public:
		BlendState(Graphics& graphics, bool isInternal);
		virtual ~BlendState();

		virtual bool AE_CALL isIndependentBlendEnabled() const override;
		virtual void AE_CALL setIndependentBlendEnabled(bool enalbed) override;

		virtual const RenderTargetBlendState& AE_CALL getRenderTargetState(uint8_t index) const override;
		virtual void AE_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) override;

		inline ID3D11BlendState1* AE_CALL getInternalState() const {
			return _internalState;
		}

		inline const uint64_t& AE_CALL getFeatureValue() const {
			return _featureValue;
		}

		void AE_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType INDEPENDENT_BLEND_ENABLE = 0b1 << 1;
			static const DirtyType RT_STATE = 0b1 << 2;
		};

		static const RenderTargetBlendState DEFAULT_RT_STATE;
		static const uint8_t MAX_RTS = 8;

		bool _isInternal;
		DirtyType _dirty;
		bool _oldIndependentBlendEnabled;
		D3D11_BLEND_DESC1 _desc;
		RenderTargetBlendState _rtStatus[MAX_RTS];
		RenderTargetBlendState _oldRtStatus[MAX_RTS];
		ID3D11BlendState1* _internalState;
		uint64_t _featureValue;

		static D3D11_BLEND AE_CALL _convertBlendFactor(BlendFactor factor);
		static D3D11_BLEND_OP AE_CALL _convertBlendOp(BlendOp op);

		void AE_CALL _releaseRes();

		void AE_CALL _setRenderTargetState(uint8_t index, const RenderTargetBlendState& state);
		inline void AE_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}