#include "RenderTarget.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_d3d11 {
	RenderTarget::RenderTarget(Graphics& graphics) : IRenderTarget(graphics) {
	}

	RenderTarget::~RenderTarget() {
	}

	const void* RenderTarget::getNative() const {
		return this;
	}

	IRenderView* RenderTarget::getRenderView(uint8_t index) const {
		return index < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ? _views[index].get() : nullptr;
	}

	bool RenderTarget::setRenderView(uint8_t index, IRenderView* view) {
		if (index < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) {
			_views[index] = view;
			return true;
		}
		return false;
	}

	void RenderTarget::eraseRenderViews(uint8_t begin, uint8_t size) {
		uint8_t n = begin + size;
		if (n > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) n = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
		for (uint8_t i = 0; i < n; ++i) _views[i].reset();
	} 

	IDepthStencil* RenderTarget::getDepthStencil() const {
		return _ds.get();
	}

	void RenderTarget::setDepthStencil(IDepthStencil* ds) {
		_ds = ds;
	}
}