#pragma once

#include "srk/Image.h"

namespace srk::extensions {
	class SRK_EXTENSION_DLL JPEGConverter {
	public:
		static constexpr uint32_t HEADER_MAGIC = 0xFFD8FF;

		static IntrusivePtr<Image> SRK_CALL decode(const ByteArray& source);
		//static ByteArray SRK_CALL encode(const Image& img);
	};
}