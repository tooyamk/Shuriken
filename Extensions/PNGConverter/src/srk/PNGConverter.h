#pragma once

#include "srk/Image.h"

namespace srk::extensions {
	class SRK_EXTENSION_DLL PNGConverter {
	public:
		static IntrusivePtr<Image> SRK_CALL decode(const ByteArray& source);
		static ByteArray SRK_CALL encode(const Image& img);
	};
}