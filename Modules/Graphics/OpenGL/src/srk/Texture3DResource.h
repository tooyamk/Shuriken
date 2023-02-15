#pragma once

#include "BaseTexture.h"

namespace srk::modules::graphics::gl {
	class SRK_MODULE_DLL Texture3DResource : public ITexture3DResource {
	public:
		Texture3DResource(Graphics& graphics);
		virtual ~Texture3DResource();

		virtual TextureType SRK_CALL getType() const override;
		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual SampleCount SRK_CALL getSampleCount() const override;
		virtual TextureFormat SRK_CALL getFormat() const override;
		virtual const Vec3uz& SRK_CALL getDimensions() const override;
		virtual bool SRK_CALL create(const Vec3uz& dim, size_t arraySize, size_t mipLevels, TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void* const* data = nullptr) override;
		virtual Usage SRK_CALL getUsage() const override;
		virtual Usage SRK_CALL map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage) override;
		virtual void SRK_CALL unmap(size_t arraySlice, size_t mipSlice) override;
		virtual size_t SRK_CALL read(size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen) override;
		virtual size_t SRK_CALL write(size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length) override;
		virtual void SRK_CALL destroy() override;
		virtual bool SRK_CALL update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data) override;
		virtual bool SRK_CALL copyFrom(const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange) override;

	protected:
		BaseTexture _baseTex;
	};
}