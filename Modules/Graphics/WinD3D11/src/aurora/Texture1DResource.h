#pragma once

#include "BaseTextureResource.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Texture1DResource : public ITexture1DResource {
	public:
		Texture1DResource(Graphics& graphics);
		virtual ~Texture1DResource();

		virtual TextureType AE_CALL getType() const override;
		virtual bool AE_CALL isCreated() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual SampleCount AE_CALL getSampleCount() const override;
		virtual uint16_t AE_CALL getPerPixelByteSize() const override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual bool AE_CALL create(uint32_t width, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) override;
		virtual void AE_CALL unmap(uint32_t arraySlice, uint32_t mipSlice) override;
		virtual uint32_t AE_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) override;
		virtual void AE_CALL destroy() override;
		virtual bool AE_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box1ui32& range, const void* data) override;
		virtual bool AE_CALL copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box1ui32& range, const IPixelBuffer* pixelBuffer) override;

	protected:
		BaseTextureResource _baseTexRes;
	};
}