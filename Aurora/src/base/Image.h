#pragma once

#include "base/ByteArray.h"

namespace aurora {
	class AE_DLL Image {
	public:
		Image();
		Image(Image&& img);
		Image& operator=(Image&& img);

		ui32 width;
		ui32 height;
		ByteArray source;
	};
}