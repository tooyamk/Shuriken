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

		inline ID3D11RasterizerState2* SRK_CALL getInternalState() const {
			return _internalState;
		}

		inline uint32_t SRK_CALL getFeatureValue() const {
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
		};

		bool _isInternal;
		DirtyType _dirty;
		FillMode _fillMode;
		FillMode _oldFillMode;
		CullMode _cullMode;
		CullMode _oldCullMode;
		FrontFace _frontFace;
		FrontFace _oldFrontFace;
		D3D11_RASTERIZER_DESC2 _desc;
		ID3D11RasterizerState2* _internalState;
		uint32_t _featureValue;

		static D3D11_FILL_MODE SRK_CALL _convertFillMode(FillMode mode);
		static D3D11_CULL_MODE SRK_CALL _convertCullMode(CullMode mode);

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