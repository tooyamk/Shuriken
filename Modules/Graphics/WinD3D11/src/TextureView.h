#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL TextureView : public ITextureView {
	public:
		TextureView(Graphics& graphics);
		virtual ~TextureView();

		virtual ITextureResource* AE_CALL getResource() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual ui32 AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(ITextureResource* res, ui32 mipBegin, ui32 mipLevels) override;

	protected:
		ui32 _mipBegin;
		ui32 _mipLevels;
		ui32 _createdMipLevels;

		RefPtr<ITextureResource> _res;
		ID3D11ShaderResourceView* _view;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
		void AE_CALL _setRes(ITextureResource* res);
		void AE_CALL _release();
		void AE_CALL _onResRecreated();
	};
}