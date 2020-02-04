#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL TextureView : public ITextureView {
	public:
		TextureView(Graphics& graphics);
		virtual ~TextureView();

		virtual bool AE_CALL isCreated() const override;
		virtual ITextureResource* AE_CALL getResource() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual uint32_t AE_CALL getArraySize() const override;
		virtual uint32_t AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void AE_CALL destroy() override;

		inline ID3D11ShaderResourceView* AE_CALL getInternalView() {
			return _view;
		}

	protected:
		uint32_t _mipBegin;
		uint32_t _mipLevels;
		uint32_t _createdMipLevels;
		uint32_t _arrayBegin;
		uint32_t _arraySize;
		uint32_t _createdArraySize;

		RefPtr<ITextureResource> _res;
		ID3D11ShaderResourceView* _view;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}