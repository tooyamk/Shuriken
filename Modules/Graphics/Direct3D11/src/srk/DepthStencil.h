#pragma once

#include "Base.h"

namespace srk::modules::graphics::d3d11 {
	class Graphics;

	class SRK_MODULE_DLL DepthStencil : public IDepthStencil {
	public:
		DepthStencil(Graphics& graphics, bool internalView);
		virtual ~DepthStencil();

		virtual const void* SRK_CALL getNative() const override;
		virtual SampleCount SRK_CALL getSampleCount() const override;
		virtual const Vec2uz& SRK_CALL getSize() const override;
		virtual bool SRK_CALL create(const Vec2uz& size, DepthStencilFormat format, SampleCount sampleCount) override;
		virtual void SRK_CALL destroy() override;


		inline ID3D11DepthStencilView* getInternalView() {
			return _view;
		}

	private:
		bool _isInternal;
		SampleCount _sampleCount;
		Vec2uz _size;

		ID3D11DepthStencilView* _view;

		static DXGI_FORMAT SRK_CALL convertInternalFormat(DepthStencilFormat fmt);
	};
}