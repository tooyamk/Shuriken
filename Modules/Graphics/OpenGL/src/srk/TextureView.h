#pragma once

#include "BaseTextureView.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL TextureView : public ITextureView {
	public:
		TextureView(Graphics& graphics);
		virtual ~TextureView();

		virtual bool SRK_CALL isCreated() const override;
		virtual IntrusivePtr<ITextureResource> SRK_CALL getResource() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual uint32_t SRK_CALL getArraySize() const override;
		virtual uint32_t SRK_CALL getMipLevels() const override;
		virtual bool SRK_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void SRK_CALL destroy() override;

	protected:
		BaseTextureView _base;

		bool SRK_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}