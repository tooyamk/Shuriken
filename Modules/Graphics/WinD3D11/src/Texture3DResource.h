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
		virtual uint16_t AE_CALL getPerPixelByteSize() const override;
		virtual uint32_t AE_CALL getArraySize() const override;
		virtual uint32_t AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(const Vec3ui32& size, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr);
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map (uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) override;
		virtual void AE_CALL unmap (uint32_t arraySlice, uint32_t mipSlice) override;
		virtual uint32_t AE_CALL read (uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write (uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) override;
		virtual bool AE_CALL update (uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const void* data) override;
		virtual bool AE_CALL copyFrom (uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) override;

	protected:
		BaseTextureResource _baseTexRes;
		TextureView _view;
	};
}