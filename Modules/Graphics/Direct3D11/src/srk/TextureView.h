#pragma once

#include "Base.h"

namespace srk::modules::graphics::d3d11 {
	class Graphics;

	class SRK_MODULE_DLL TextureView : public ITextureView {
	public:
		TextureView(Graphics& graphics);
		virtual ~TextureView();

		virtual bool SRK_CALL isCreated() const override;
		virtual IntrusivePtr<ITextureResource> SRK_CALL getResource() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual size_t SRK_CALL getArraySize() const override;
		virtual size_t SRK_CALL getMipLevels() const override;
		virtual bool SRK_CALL create(ITextureResource* res, size_t mipBegin, size_t mipLevels, size_t arrayBegin, size_t arraySize) override;
		virtual void SRK_CALL destroy() override;

		inline ID3D11ShaderResourceView* SRK_CALL getInternalView() {
			return _view;
		}

	protected:
		size_t _mipBegin;
		size_t _mipLevels;
		size_t _createdMipLevels;
		size_t _arrayBegin;
		size_t _arraySize;
		size_t _createdArraySize;

		IntrusivePtr<ITextureResource> _res;
		ID3D11ShaderResourceView* _view;

		bool SRK_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}