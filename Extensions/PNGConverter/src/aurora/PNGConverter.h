#pragma once

#include "aurora/Image.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL PNGConverter {
	public:
		AE_DECLARE_CANNOT_INSTANTIATE(PNGConverter);

		static Image* AE_CALL parse(const ByteArray& source);
		static ByteArray AE_CALL encode(const Image& img);
	};
}