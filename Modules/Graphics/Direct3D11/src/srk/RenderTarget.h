#pragma once

#include "Base.h"

namespace srk::modules::graphics::d3d11 {
	class Graphics;

	class SRK_MODULE_DLL RenderTarget : public IRenderTarget {
	public:
		RenderTarget(Graphics& graphics);
		virtual ~RenderTarget();

		virtual const void* SRK_CALL getNative() const override;
		
		virtual Vec2ui32 SRK_CALL getSize() const override;
		virtual IntrusivePtr<IRenderView> SRK_CALL getRenderView(uint8_t index) const override;
		virtual bool SRK_CALL setRenderView(uint8_t index, IRenderView* view) override;
		virtual void SRK_CALL eraseRenderViews(uint8_t begin, uint8_t size) override;

		virtual IntrusivePtr<IDepthStencil> SRK_CALL getDepthStencil() const override;
		virtual void SRK_CALL setDepthStencil(IDepthStencil* ds) override;

		uint8_t SRK_CALL getNumRenderViews();

	protected:
		bool _numViewsDirty;
		uint8_t _numViews;

		IntrusivePtr<IRenderView> _views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		IntrusivePtr<IDepthStencil> _ds;
	};
}