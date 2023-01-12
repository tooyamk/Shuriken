#pragma once

#include "BaseTextureView.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL TextureViewSimulative : public ITextureView {
	public:
		TextureViewSimulative(Graphics& graphics);
		virtual ~TextureViewSimulative();

		virtual bool SRK_CALL isCreated() const override;
		virtual IntrusivePtr<ITextureResource> SRK_CALL getResource() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual size_t SRK_CALL getArraySize() const override;
		virtual size_t SRK_CALL getMipLevels() const override;
		virtual bool SRK_CALL create(ITextureResource* res, size_t mipBegin, size_t mipLevels, size_t arrayBegin, size_t arraySize) override;
		virtual void SRK_CALL destroy() override;

	protected:
		BaseTextureView _base;

		bool SRK_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}