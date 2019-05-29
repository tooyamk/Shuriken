#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_glew {
	class Graphics;

	class AE_MODULE_DLL TextureView : public ITextureView {
	public:
		TextureView(Graphics& graphics);
		virtual ~TextureView();

		virtual ITextureResource* AE_CALL getResource() const override;
		virtual const void* AE_CALL getNativeView() const override;
		virtual ui32 AE_CALL getArraySize() const override;
		virtual ui32 AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(ITextureResource* res, ui32 mipBegin, ui32 mipLevels, ui32 arrayBegin, ui32 arraySize) override;

		void AE_CALL release();

	protected:
		ui32 _mipBegin;
		ui32 _mipLevels;
		ui32 _createdMipLevels;
		ui32 _arrayBegin;
		ui32 _arraySize;
		ui32 _createdArraySize;

		RefPtr<ITextureResource> _res;
		GLuint _handle;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
		void AE_CALL _setRes(ITextureResource* res);
		void AE_CALL _onResRecreated();
	};
}