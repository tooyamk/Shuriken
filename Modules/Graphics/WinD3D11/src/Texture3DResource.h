#pragma once

#include "BaseTextureResource.h"
#include "TextureView.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Texture3DResource : public ITexture3DResource {
	public:
		Texture3DResource(Graphics& graphics);
		virtual ~Texture3DResource();

		virtual TextureType AE_CALL getType() const override;
		virtual const void* AE_CALL getNativeView() const override;
		virtual const void* AE_CALL getNativeResource() const override;
		virtual ui32 AE_CALL getArraySize() const override;
		virtual ui32 AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(const Vec3ui32& size, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr);
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) override;
		virtual void AE_CALL unmap(ui32 arraySlice, ui32 mipSlice) override;
		virtual i32 AE_CALL read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen, i32 readLen = -1) override;
		virtual i32 AE_CALL write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) override;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box3ui32& range, const void* data) ;

	protected:
		BaseTextureResource _baseTexRes;
		TextureView _view;
	};
}