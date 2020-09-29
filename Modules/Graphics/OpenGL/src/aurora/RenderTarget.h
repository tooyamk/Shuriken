#pragma once

#include "Base.h"

namespace aurora::modules::graphics::gl {
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

		void AE_CALL update();

		inline GLuint AE_CALL getInternalBuffer() {
			return _handle;
		}

	protected:
		bool _numViewsDirty;
		uint8_t _numViews;

		std::vector<RefPtr<IRenderView>> _views;
		std::vector<GLuint> _bindingViews;
		RefPtr<IDepthStencil> _ds;
		GLenum _bindingDSAttachmentType;
		GLuint _bindingDSBuffer;

		GLuint _handle;

		inline void AE_CALL _unbindDS() {
			if (_bindingDSBuffer) {
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, _bindingDSAttachmentType, GL_RENDERBUFFER, 0);
				_bindingDSBuffer = 0;
				_bindingDSAttachmentType = GL_NONE;
			}
		}
	};
}