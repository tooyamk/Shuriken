#pragma once

#include "base/Image.h"

namespace aurora::file {
	class AE_EXTENSION_DLL PNG {
	public:
		static const ui32 CHUNK_IHDR = 0x49484452;
		static const ui32 CHUNK_PLTE = 0x504C5445;
		static const ui32 CHUNK_TRNS = 0x74524E53;
		static const ui32 CHUNK_IDAT = 0x49444154;

		static const ui8 COLOR_GRAY = 0;
		static const ui8 COLOR_RGB = 2;
		static const ui8 COLOR_INDEX_RGB = 3;
		static const ui8 COLOR_ALPHA_GRAY = 4;
		static const ui8 COLOR_ARGB = 6;

		static const ui8 FILTER_NONE = 0;
		static const ui8 FILTER_SUB = 1;
		static const ui8 FILTER_UP = 2;
		static const ui8 FILTER_AVERAGE = 3;
		static const ui8 FILTER_PAETHt = 4;

		static Image parse(ByteArray& source);
	};
}