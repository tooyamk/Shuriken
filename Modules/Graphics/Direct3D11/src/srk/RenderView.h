#pragma once

#include "Base.h"

namespace srk::modules::graphics::d3d11 {
	class Graphics;

	class SRK_MODULE_DLL RenderView : public IRenderView {
	public:
		RenderView(Graphics& graphics);
		virtual ~RenderView();

		virtual bool SRK_CALL isCreated() const override;
		virtual IntrusivePtr<ITextureResource> SRK_CALL getResource() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual size_t SRK_CALL getArraySize() const override;
		virtual size_t SRK_CALL getMipSlice() const override;
		virtual bool SRK_CALL create(ITextureResource* res, size_t mipSlice, size_t arrayBegin, size_t arraySize) override;
		virtual void SRK_CALL destroy() override;

		inline ID3D11RenderTargetView1* SRK_CALL getInternalView() {
			return _view;
		}

	protected:
		size_t _mipSlice;
		size_t _arrayBegin;
		size_t _arraySize;
		size_t _createdArraySize;

		IntrusivePtr<ITextureResource> _res;
		ID3D11RenderTargetView1* _view;

		bool SRK_CALL _createDone(bool succeeded, ITextureResource* res);
	};
}