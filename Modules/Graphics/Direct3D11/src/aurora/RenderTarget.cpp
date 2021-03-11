#include "RenderTarget.h"
#include "Graphics.h"

namespace aurora::modules::graphics::d3d11 {
	RenderTarget::RenderTarget(Graphics& graphics) : IRenderTarget(graphics),
		_numViewsDirty(true),
		_numViews(0) {
	}

	RenderTarget::~RenderTarget() {
	}

	const void* RenderTarget::getNative() const {
		return this;
	}

	Vec2ui32 RenderTarget::getSize() const {
		Vec2ui32 size;

		for (size_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
			if (_views[i]) {
				if (auto res = _views[i]->getResource(); res && res->isCreated()) {
					size = res->getSize();
					break;
				}
			}
		}

		return size;
	}

	IntrusivePtr<IRenderView> RenderTarget::getRenderView(uint8_t index) const {
		return index < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ? _views[index].get() : nullptr;
	}

	bool RenderTarget::setRenderView(uint8_t index, IRenderView* view) {
		if (index < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) {
			_views[index] = view;
			if (!_numViewsDirty) {
				if (view) {
					if (_numViews <= index) _numViewsDirty = true;
				} else {
					if (_numViews == index + 1) _numViewsDirty = true;
				}
			}

			return true;
		}
		return false;
	}

	void RenderTarget::eraseRenderViews(uint8_t begin, uint8_t size) {
		if (size) {
			uint8_t n = begin + size;
			if (n > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) n = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
			for (uint8_t i = 0; i < n; ++i) _views[i].reset();
			if (!_numViewsDirty && _numViews >= begin + 1) _numViewsDirty = true;
		}
	} 

	IntrusivePtr<IDepthStencil> RenderTarget::getDepthStencil() const {
		return _ds;
	}

	void RenderTarget::setDepthStencil(IDepthStencil* ds) {
		_ds = ds;
	}

	uint8_t RenderTarget::getNumRenderViews() {
		if (_numViewsDirty) {
			_numViewsDirty = false;

			if constexpr ((bool)D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) {
				uint8_t i = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT - 1;
				do {
					if (_views[i]) {
						_numViews = i + 1;
						break;
					}

					if (i) {
						--i;
					} else {
						_numViews = 0;
						break;
					}
				} while (true);
			} else {
				_numViews = 0;
			}
		}

		return _numViews;
	}
}