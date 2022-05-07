#pragma once

#include "BaseTextureResource.h"

namespace srk::modules::graphics::d3d11 {
	class SRK_MODULE_DLL Texture1DResource : public ITexture1DResource {
	public:
		Texture1DResource(Graphics& graphics);
		virtual ~Texture1DResource();

		virtual TextureType SRK_CALL getType() const override;
		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual SampleCount SRK_CALL getSampleCount() const override;
		virtual uint16_t SRK_CALL getPerPixelByteSize() const override;
		virtual TextureFormat SRK_CALL getFormat() const override;
		virtual const Vec3ui32& SRK_CALL getSize() const override;
		virtual bool SRK_CALL create(uint32_t width, uint32_t arraySize, uint32_t mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) override;
		virtual Usage SRK_CALL getUsage() const override;
		virtual Usage SRK_CALL map(uint32_t arraySlice, uint32_t mipSlice, Usage expectMapUsage) override;
		virtual void SRK_CALL unmap(uint32_t arraySlice, uint32_t mipSlice) override;
		virtual uint32_t SRK_CALL read(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t SRK_CALL write(uint32_t arraySlice, uint32_t mipSlice, uint32_t offset, const void* data, uint32_t length) override;
		virtual void SRK_CALL destroy() override;
		virtual bool SRK_CALL update(uint32_t arraySlice, uint32_t mipSlice, const Box1ui32& range, const void* data) override;
		virtual bool SRK_CALL copyFrom(const Vec3ui32& dstPos, uint32_t dstArraySlice, uint32_t dstMipSlice, const ITextureResource* src, uint32_t srcArraySlice, uint32_t srcMipSlice, const Box3ui32& srcRange) override;
		virtual bool SRK_CALL copyFrom(uint32_t arraySlice, uint32_t mipSlice, const Box3ui32& range, const IPixelBuffer* pixelBuffer) override;
		virtual bool SRK_CALL copyTo(uint32_t mipSlice, const IPixelBuffer* pixelBuffer) override;

	protected:
		BaseTextureResource _baseTexRes;
	};
}