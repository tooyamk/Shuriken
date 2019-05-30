#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL TextureView : public ITextureView {
	public:
		TextureView(Graphics& graphics, bool internalView);
		virtual ~TextureView();

		virtual ITextureResource* AE_CALL getResource() const override;
		virtual const void* AE_CALL getNativeView() const override;
		virtual ui32 AE_CALL getArraySize() const override;
		virtual ui32 AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(ITextureResource* res, ui32 mipBegin, ui32 mipLevels, ui32 arrayBegin, ui32 arraySize) override;

		void AE_CALL release();

		inline void AE_CALL onResRecreated() {
			create(_res.get(), _mipBegin, _mipLevels, _arrayBegin, _arraySize);
		}

	protected:
		bool _internalView;
		ui32 _mipBegin;
		ui32 _mipLevels;
		ui32 _createdMipLevels;
		ui32 _arrayBegin;
		ui32 _arraySize;
		ui32 _createdArraySize;

		RefPtr<ITextureResource> _res;
		ID3D11ShaderResourceView* _view;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
		void AE_CALL _setRes(ITextureResource* res);
	};
}