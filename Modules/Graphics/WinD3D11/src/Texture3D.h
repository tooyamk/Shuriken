#pragma once

#include "BaseTexture.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Texture3D : public ITexture3D {
	public:
		Texture3D(Graphics& graphics);
		virtual ~Texture3D();

		virtual TextureType AE_CALL getType() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL allocate(ui32 width, ui32 height, ui32 depth, TextureFormat format, ui32 mipLevels, Usage resUsage, const void*const* data = nullptr) ;
		virtual Usage AE_CALL map(ui32 mipLevel, Usage expectMapUsage) override;
		virtual void AE_CALL unmap(ui32 mipLevel) override;
		virtual i32 AE_CALL read(ui32 mipLevel, ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 mipLevel, ui32 offset, const void* data, ui32 length) override;
		virtual bool AE_CALL write(ui32 mipLevel, const Box<ui32>& range, const void* data) ;

	protected:
		BaseTexture _baseTex;
	};
}