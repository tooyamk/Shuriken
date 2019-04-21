#pragma once

#include "BaseResource.h"

namespace aurora::modules::graphics::win_d3d11 {
	class AE_MODULE_DLL Texture2D : private BaseResource, public ITexture2D {
	public:
		Texture2D(Graphics& graphics);
		virtual ~Texture2D();

		virtual TextureType AE_CALL getType() const override;
		virtual bool AE_CALL allocate(ui32 width, ui32 height, TextureFormat format, ui32 bufferUsage, const void* data = nullptr) override;

	protected:
	};
}