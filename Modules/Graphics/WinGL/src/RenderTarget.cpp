#include "RenderTarget.h"
#include "DepthStencil.h"
#include "RenderView.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_gl {
	RenderTarget::RenderTarget(Graphics& graphics) : IRenderTarget(graphics),
		_numViewsDirty(true),
		_numViews(0),
		_views(graphics.getDeviceFeatures().simultaneousRenderTargetCount),
		_bindedViews(_views.size(), 0),
		_bindedDSBuffer(0),
		_bindedDSAttachmentType(GL_NONE),
		_handle(0) {
		glGenFramebuffers(1, &_handle);
	}

	RenderTarget::~RenderTarget() {
		if (_handle) {
			glDeleteFramebuffers(1, &_handle);
			_handle = 0;
		}
	}

	const void* RenderTarget::getNative() const {
		return this;
	}

	IRenderView* RenderTarget::getRenderView(uint8_t index) const {
		return index < _views.size() ? _views[index].get() : nullptr;
	}

	bool RenderTarget::setRenderView(uint8_t index, IRenderView* view) {
		if (index < _views.size()) {
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
		uint8_t n = begin + size;
		if (n > _views.size()) n = _views.size();
		for (uint8_t i = 0; i < n; ++i) _views[i].reset();
		if (!_numViewsDirty && _numViews >= begin + 1) _numViewsDirty = true;
	} 

	IDepthStencil* RenderTarget::getDepthStencil() const {
		return _ds.get();
	}

	void RenderTarget::setDepthStencil(IDepthStencil* ds) {
		_ds = ds;
	}

	void RenderTarget::update() {
		auto oldNumViews = _numViews;

		if (_numViewsDirty) {
			_numViewsDirty = false;

			if (auto size = _views.size(); size) {
				uint8_t i = size - 1;
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

		GLuint bindedFB = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&bindedFB);
		glBindFramebuffer(GL_FRAMEBUFFER, _handle);

		for (uint8_t i = 0; i < _numViews; ++i) {
			BaseRenderView* base = nullptr;
			if (_views[i]) base = (BaseRenderView*)_views[i]->getNative();

			if (base && base->handle) {
				if (_bindedViews[i] != base->handle) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, base->internalFormat, base->handle, 0);
					_bindedViews[i] = base->handle;
				}
			} else {
				if (_bindedViews[i]) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
					_bindedViews[i] = 0;
				}
			}
		}
		if (oldNumViews > _numViews) {
			for (uint8_t i = _numViews; i < oldNumViews; ++i) {
				if (_bindedViews[i]) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
					_bindedViews[i] = 0;
				}
			}
		}

		if (_ds) {
			if (auto native = (DepthStencil*)_ds->getNative(); native) {
				if (auto buf = native->getInternalBuffer(); buf) {
					if (_bindedDSBuffer != buf || _bindedDSAttachmentType != native->getInternalAttachmentType()) {
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, native->getInternalAttachmentType(), GL_RENDERBUFFER, buf);
						_bindedDSBuffer = buf;
						_bindedDSAttachmentType = native->getInternalAttachmentType();
					}
				} else {
					_unbindDS();
				}
			} else {
				_unbindDS();
			}
		} else {
			_unbindDS();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, bindedFB);
	}
}