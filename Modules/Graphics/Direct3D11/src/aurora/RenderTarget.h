#pragma once

#include "Base.h"

namespace aurora::modules::graphics::d3d11 {
	class Graphics;

	class AE_MODULE_DLL RenderTarget : public IRenderTarget {
	public:
		RenderTarget(Graphics& graphics);
		virtual ~RenderTarget();

		virtual const void* AE_CALL getNative() const override;
		
		virtual Vec2ui32 AE_CALL getSize() const override;
		virtual RefPtr<IRenderView> AE_CALL getRenderView(uint8_t index) const override;
		virtual bool AE_CALL setRenderView(uint8_t index, IRenderView* view) override;
		virtual void AE_CALL eraseRenderViews(uint8_t begin, uint8_t size) override;

		virtual RefPtr<IDepthStencil> AE_CALL getDepthStencil() const override;
		virtual void AE_CALL setDepthStencil(IDepthStencil* ds) override;

		uint8_t AE_CALL getNumRenderViews();

	protected:
		bool _numViewsDirty;
		uint8_t _numViews;

		RefPtr<IRenderView> _views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		RefPtr<IDepthStencil> _ds;
	};
}