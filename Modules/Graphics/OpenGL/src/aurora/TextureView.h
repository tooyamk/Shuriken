#pragma once

#include "BaseTextureView.h"

namespace aurora::modules::graphics::gl {
	class Graphics;

	class AE_MODULE_DLL TextureView : public ITextureView {
	public:
		TextureView(Graphics& graphics);
		virtual ~TextureView();

		virtual bool AE_CALL isCreated() const override;
		virtual IntrusivePtr<ITextureResource> AE_CALL getResource() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual uint32_t AE_CALL getArraySize() const override;
		virtual uint32_t AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void AE_CALL destroy() override;

	protected:
		BaseTextureView _base;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}