#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
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

		inline const InternalDepthState& SRK_CALL getInternalDepthState() const {
			return _internalDepthState;
		}

		inline const InternalStencilState& SRK_CALL getInternalStencilState() const {
			return _internalStencilState;
		}

		inline const DepthStencilFeature& SRK_CALL getStencilFeatureValue() const {
			return _stencilFeatureValue;
		}

		void SRK_CALL update();

	protected:
		using DirtyType = uint8_t;

		struct DirtyFlag {
			static const DirtyType EMPTY = 0b1;
			static const DirtyType DEPTH = 0b1 << 1;
			static const DirtyType STENCIL = 0b1 << 2;
		};

		bool _isInternal;
		DirtyType _dirty;
		DepthState _depthState;
		DepthState _oldDepthState;
		StencilState _stencilState;
		StencilState _oldStencilState;
		InternalDepthState _internalDepthState;
		InternalStencilState _internalStencilState;
		DepthStencilFeature _stencilFeatureValue;

		void SRK_CALL _updateDepth();
		void SRK_CALL _updateStencil();
		void SRK_CALL _updateStecnilFace(InternalStencilFaceState& desc, const StencilFaceState& state);

		inline void SRK_CALL _setDirty(bool dirty, DirtyType val) {
			if (dirty) {
				_dirty |= val;
			} else {
				_dirty &= ~val;
			}
		}
	};
}