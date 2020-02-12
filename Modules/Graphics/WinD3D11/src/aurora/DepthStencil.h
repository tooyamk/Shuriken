#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL DepthStencil : public IDepthStencil {
	public:
		DepthStencil(Graphics& graphics, bool internalView);
		virtual ~DepthStencil();

		virtual const void* AE_CALL getNative() const override;
		virtual SampleCount AE_CALL getSampleCount() const override;
		virtual const Vec2ui32& AE_CALL getSize() const override;
		virtual bool AE_CALL create(const Vec2ui32& size, DepthStencilFormat format, SampleCount sampleCount) override;
		virtual void AE_CALL destroy() override;


		inline ID3D11DepthStencilView* getInternalView() {
			return _view;
		}

	private:
		bool _isInternal;
		SampleCount _sampleCount;
		Vec2ui32 _size;

		ID3D11DepthStencilView* _view;

		static DXGI_FORMAT AE_CALL convertInternalFormat(DepthStencilFormat fmt);
	};
}