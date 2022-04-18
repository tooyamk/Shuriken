#pragma once

#include "Base.h"

namespace aurora::modules::graphics::d3d11 {
	class Graphics;

	class AE_MODULE_DLL BlendState : public IBlendState {
	public:
		BlendState(Graphics& graphics, bool isInternal);
		virtual ~BlendState();

		virtual const void* AE_CALL getNative() const override;
		virtual uint8_t AE_CALL getCount() const override;
		virtual void AE_CALL setCount(uint8_t count) override;

		virtual const RenderTargetBlendState* AE_CALL getRenderTargetState(uint8_t index) const override;
		virtual bool AE_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) override;

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
			static const DirtyType COUNT = 0b1 << 1;
			static const DirtyType RT_STATE = 0b1 << 2;
		};

		bool _isInternal;
		DirtyType _dirty;
		uint8_t _count;
		uint8_t _oldCount;
		D3D11_BLEND_DESC1 _desc;
		RenderTargetBlendState _rtStatus[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		RenderTargetBlendState _oldRtStatus[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
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