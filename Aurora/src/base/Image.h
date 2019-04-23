#pragma once

#include "base/ByteArray.h"
#include "modules/IGraphicsModule.h"

namespace aurora {
	class AE_DLL Image : public Ref {
	public:
		Image();

		modules::graphics::TextureFormat format;
		ui32 width;
		ui32 height;
		ByteArray source;
	};
}