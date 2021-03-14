#pragma once

#include "aurora/Image.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL PNGConverter {
	public:
		static IntrusivePtr<Image> AE_CALL decode(const ByteArray& source);
		static ByteArray AE_CALL encode(const Image& img);
	};
}