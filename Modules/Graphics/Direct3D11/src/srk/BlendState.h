#pragma once

#include "Base.h"

namespace srk::modules::graphics::d3d11 {
	class Graphics;

	class SRK_MODULE_DLL BlendState : public IBlendState {
	public:
		BlendState(Graphics& graphics, bool isInternal);
		virtual ~BlendState();

		virtual const void* SRK_CALL getNative() const override;
		virtual const Vec4f32& SRK_CALL getConstants() const override;
		virtual void SRK_CALL setConstants(const Vec4f32& val) override;
		virtual uint8_t SRK_CALL getCount() const override;
		virtual void SRK_CALL setCount(uint8_t count) override;

		virtual const RenderTargetBlendState* SRK_CALL getRenderTargetState(uint8_t index) const override;
		virtual bool SRK_CALL setRenderTargetState(uint8_t index, const RenderTargetBlendState& state) override;

		inline ID3D11BlendState1* SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline const uint64_t& SRK_CALL getFeatureValue() const {
			return _featureValue;
		}

		void SRK_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType COUNT = 0b1 << 1;
			static const DirtyType CONSTANTS = 0b1 << 2;
			static const DirtyType RT_STATE = 0b1 << 3;
		};

		bool _isInternal;
		DirtyType _dirty;
		uint8_t _count;
		uint8_t _oldCount;
		Vec4f32 _constants;
		D3D11_BLEND_DESC1 _desc;
		RenderTargetBlendState _rtStatus[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		RenderTargetBlendState _oldRtStatus[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		ID3D11BlendState1* _internalState;
		uint64_t _featureValue;

		void SRK_CALL _releaseRes();

		void SRK_CALL _setRenderTargetState(uint8_t index, const RenderTargetBlendState& state);
		inline void SRK_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}