#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL RenderView : public IRenderView {
	public:
		RenderView(Graphics& graphics);
		virtual ~RenderView();

		virtual bool AE_CALL isCreated() const override;
		virtual ITextureResource* AE_CALL getResource() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual uint32_t AE_CALL getArraySize() const override;
		virtual uint32_t AE_CALL getMipSlice() const override;
		virtual bool AE_CALL create(ITextureResource* res, uint32_t mipSlice, uint32_t arrayBegin, uint32_t arraySize) override;
		virtual void AE_CALL destroy() override;

		inline ID3D11RenderTargetView1* AE_CALL getInternalView() {
			return _view;
		}

	protected:
		uint32_t _mipSlice;
		uint32_t _arrayBegin;
		uint32_t _arraySize;
		uint32_t _createdArraySize;

		RefPtr<ITextureResource> _res;
		ID3D11RenderTargetView1* _view;

		bool AE_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}