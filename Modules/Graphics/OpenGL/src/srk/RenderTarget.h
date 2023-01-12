#pragma once

#include "Base.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL RenderTarget : public IRenderTarget {
	public:
		RenderTarget(Graphics& graphics);
		virtual ~RenderTarget();

		virtual const void* SRK_CALL getNative() const override;
		
		virtual Vec2uz SRK_CALL getDimensions() const override;
		virtual IntrusivePtr<IRenderView> SRK_CALL getRenderView(uint8_t index) const override;
		virtual bool SRK_CALL setRenderView(uint8_t index, IRenderView* view) override;
		virtual void SRK_CALL eraseRenderViews(uint8_t begin, uint8_t count) override;

		virtual IntrusivePtr<IDepthStencil> SRK_CALL getDepthStencil() const override;
		virtual void SRK_CALL setDepthStencil(IDepthStencil* ds) override;

		void SRK_CALL update();

		inline GLuint SRK_CALL getInternalBuffer() {
			return _handle;
		}

	protected:
		bool _numViewsDirty;
		uint8_t _numViews;

		std::vector<IntrusivePtr<IRenderView>> _views;
		std::vector<GLuint> _bindingViews;
		IntrusivePtr<IDepthStencil> _ds;
		GLenum _bindingDSAttachmentType;
		GLuint _bindingDSBuffer;

		GLuint _handle;

		inline void SRK_CALL _unbindDS() {
			if (_bindingDSBuffer) {
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, _bindingDSAttachmentType, GL_RENDERBUFFER, 0);
				_bindingDSBuffer = 0;
				_bindingDSAttachmentType = GL_NONE;
			}
		}
	};
}