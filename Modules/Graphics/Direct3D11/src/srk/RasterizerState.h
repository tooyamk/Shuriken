#pragma once

#include "Base.h"

namespace srk::modules::graphics::d3d11 {
	class Graphics;

	class SRK_MODULE_DLL RasterizerState : public IRasterizerState {
	public:
		RasterizerState(Graphics& graphics, bool isInternal);
		virtual ~RasterizerState();

		virtual const void* SRK_CALL getNative() const override;

		virtual FillMode SRK_CALL getFillMode() const override;
		virtual void SRK_CALL setFillMode(FillMode fill) override;

		virtual CullMode SRK_CALL getCullMode() const override;
		virtual void SRK_CALL setCullMode(CullMode cull) override;

		virtual FrontFace SRK_CALL getFrontFace() const override;
		virtual void SRK_CALL setFrontFace(FrontFace front) override;

		virtual bool SRK_CALL getScissorEnabled() const override;
		virtual void SRK_CALL setScissorEnabled(bool enabled) override;

		inline ID3D11RasterizerState2* SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline const RasterizerFeature& SRK_CALL getFeatureValue() const {
			return _featureValue;
		}

		void SRK_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType FILL_MODE = 0b1 << 1;
			static const DirtyType CULL_MODE = 0b1 << 2;
			static const DirtyType FRONT_FACE = 0b1 << 3;
			static const DirtyType SCISSOR = 0b1 << 4;
		};

		bool _isInternal;
		DirtyType _dirty;
		RasterizerDescriptor _cur;
		RasterizerDescriptor _old;
		D3D11_RASTERIZER_DESC2 _desc;
		ID3D11RasterizerState2* _internalState;
		RasterizerFeature _featureValue;

		void SRK_CALL _releaseRes();

		inline void SRK_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}