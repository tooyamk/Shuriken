#pragma once

#include "BaseRenderView.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL RenderView : public IRenderView {
	public:
		RenderView(Graphics& graphics);
		virtual ~RenderView();

		virtual bool SRK_CALL isCreated() const override;
		virtual IntrusivePtr<ITextureResource> SRK_CALL getResource() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual uint32_t SRK_CALL getArraySize() const override;
		virtual uint32_t SRK_CALL getMipSlice() const override;
		virtual bool SRK_CALL create(ITextureResource* res, uint32_t mipSlice, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void SRK_CALL destroy() override;

	protected:
		BaseRenderView _base;

		bool SRK_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}