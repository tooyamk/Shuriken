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
		virtual uint32_t AE_CALL getArraySize() const override;
		virtual uint32_t AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipBegin, uint32_t mipLevels, uint32_t arrayBegin, uint32_t arraySize) override;

		void AE_CALL release();

		inline void AE_CALL onResRecreated() {
			create(_res.get(), _mipBegin, _mipLevels, _arrayBegin, _arraySize);
		}

	protected:
		uint32_t _mipBegin;
		uint32_t _mipLevels;
		uint32_t _createdMipLevels;
		uint32_t _arrayBegin;
		uint32_t _arraySize;
		uint32_t _createdArraySize;

		RefPtr<ITextureResource> _res;
		GLuint _handle;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
		void AE_CALL _setRes(ITextureResource* res);
	};
}