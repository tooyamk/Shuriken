#pragma once

#include "BaseRenderView.h"

namespace aurora::modules::graphics::gl {
	class Graphics;

	class AE_MODULE_DLL RenderView : public IRenderView {
	public:
		RenderView(Graphics& graphics);
		virtual ~RenderView();

		virtual bool AE_CALL isCreated() const override;
		virtual IntrusivePtr<ITextureResource> AE_CALL getResource() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual uint32_t AE_CALL getArraySize() const override;
		virtual uint32_t AE_CALL getMipSlice() const override;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipSlice, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void AE_CALL destroy() override;

	protected:
		BaseRenderView _base;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}