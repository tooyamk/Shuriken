#pragma once

#include "Base.h"

namespace aurora::modules::graphics::win_d3d11 {
	class Graphics;

	class AE_MODULE_DLL DepthStencil : public IDepthStencil {
	public:
		DepthStencil(Graphics& graphics, bool internalView);
		virtual ~DepthStencil();

		virtual bool AE_CALL isMultisampling() const override;
		virtual const Vec2ui32& AE_CALL getSize() const override;
		virtual bool AE_CALL create(const Vec2ui32& size, bool multisampling) override;


		inline ID3D11DepthStencilView* getInernalView() {
			return _view;
		}

	private:
		bool _isInternal;
		bool _isMultisampling;
		Vec2ui32 _size;
		ID3D11Texture2D* _tex;
		ID3D11DepthStencilView* _view;

		void AE_CALL _release();
	};
}