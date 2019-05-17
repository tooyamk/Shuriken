#pragma once

#include "BaseTexture.h"

namespace aurora::modules::graphics::win_glew {
	class AE_MODULE_DLL Texture2DResource : public ITexture2DResource {
	public:
		Texture2DResource(Graphics& graphics);
		virtual ~Texture2DResource();

		virtual TextureType AE_CALL getType() const override;
		virtual const void* AE_CALL getNativeView() const override;
		virtual const void* AE_CALL getNativeResource() const override;
		virtual ui32 AE_CALL getArraySize() const override;
		virtual ui32 AE_CALL getMipLevels() const override;
		virtual bool AE_CALL create(const Vec2ui32& size, ui32 arraySize, ui32 mipLevels, TextureFormat format, Usage resUsage, const void*const* data = nullptr) override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(ui32 arraySlice, ui32 mipSlice, Usage expectMapUsage) override;
		virtual void AE_CALL unmap(ui32 arraySlice, ui32 mipSlice) override;
		virtual ui32 AE_CALL read(ui32 arraySlice, ui32 mipSlice, ui32 offset, void* dst, ui32 dstLen) override;
		virtual ui32 AE_CALL write(ui32 arraySlice, ui32 mipSlice, ui32 offset, const void* data, ui32 length) override;
		virtual bool AE_CALL update(ui32 arraySlice, ui32 mipSlice, const Box2ui32& range, const void* data) override;

	protected:
		BaseTexture _baseTex;
	};
}