#pragma once

#include "base/Image.h"

namespace aurora::file {
	class AE_EXTENSION_DLL PNGConverter {
	public:
		static Image* AE_CALL parse(const ByteArray& source);
	};
}