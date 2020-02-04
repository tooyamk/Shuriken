#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL RenderTarget : public IRenderTarget {
	public:
		RenderTarget(Graphics& graphics);
		virtual ~RenderTarget();

		virtual const void* AE_CALL getNative() const override;
		
		virtual IRenderView* AE_CALL getRenderView(uint8_t index) const override;
		virtual bool AE_CALL setRenderView(uint8_t index, IRenderView* view) override;
		virtual void AE_CALL eraseRenderViews(uint8_t begin, uint8_t size) override;

		virtual IDepthStencil* AE_CALL getDepthStencil() const override;
		virtual void AE_CALL setDepthStencil(IDepthStencil* ds) override;

	protected:
		RefPtr<IRenderView> _views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		RefPtr<IDepthStencil> _ds;
	};
}