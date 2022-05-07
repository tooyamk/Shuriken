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
		virtual uint32_t SRK_CALL getArraySize() const override;
		virtual uint32_t SRK_CALL getMipLevels() const override;
		virtual bool SRK_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void SRK_CALL destroy() override;

		inline ID3D11ShaderResourceView* SRK_CALL getInternalView() {
			return _view;
		}

	protected:
		uint32_t _mipBegin;
		uint32_t _mipLevels;
		uint32_t _createdMipLevels;
		uint32_t _arrayBegin;
		uint32_t _arraySize;
		uint32_t _createdArraySize;

		IntrusivePtr<ITextureResource> _res;
		ID3D11ShaderResourceView* _view;

		bool SRK_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}