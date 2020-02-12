#pragma once

#include "aurora/Image.h"

namespace aurora::extensions::file {
	class AE_EXTENSION_DLL PNGConverter {
	public:
		static Image* AE_CALL parse(const ByteArray& source);
	};
}