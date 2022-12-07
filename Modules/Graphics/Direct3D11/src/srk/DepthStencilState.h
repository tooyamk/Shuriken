#pragma once

#include "Base.h"

namespace srk::modules::graphics::d3d11 {
	class Graphics;

	class SRK_MODULE_DLL DepthStencilState : public IDepthStencilState {
	public:
		DepthStencilState(Graphics& graphics, bool isInternal);
		virtual ~DepthStencilState();

		virtual const void* SRK_CALL getNative() const override;

		virtual const DepthState& SRK_CALL getDepthState() const override;
		virtual void SRK_CALL setDepthState(const DepthState& depthState) override;

		virtual const StencilState& SRK_CALL getStencilState() const override;
		virtual void SRK_CALL setStencilState(const StencilState& stencilState) override;

		inline ID3D11DepthStencilState* SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline const DepthStencilFeature& SRK_CALL getFeatureValue() const {
			return _featureValue;
		}

		inline uint8_t SRK_CALL getStencilRef() const {
			return _stencilState.face.front.ref;
		}

		void SRK_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType DEPTH = 0b1 << 1;
			static const DirtyType STENCIL = 0b1 << 2;
			static const DirtyType STENCIL_REF = 0b1 << 3;
		};

		bool _isInternal;
		DirtyType _dirty;
		DepthState _depthState;
		DepthState _oldDepthState;
		StencilState _stencilState;
		StencilState _oldStencilState;
		D3D11_DEPTH_STENCIL_DESC _desc;
		ID3D11DepthStencilState* _internalState;
		DepthStencilFeature _featureValue;

		static bool SRK_CALL _isStecnilBaseNotEqual(const StencilState& lhs, const StencilState& rhs);

		void SRK_CALL _releaseRes();

		void SRK_CALL _updateDepth();
		void SRK_CALL _updateStencil();
		void SRK_CALL _updateStecnilFace(D3D11_DEPTH_STENCILOP_DESC& desc, const StencilFaceState& state);

		inline void SRK_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}