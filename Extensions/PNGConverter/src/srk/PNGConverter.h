#pragma once

#include "srk/Image.h"

#ifdef SRK_EXT_PNG_CONV_EXPORTS
#	define SRK_EXT_PNG_CONV_DLL SRK_DLL_EXPORT
#else
#	define SRK_EXT_PNG_CONV_DLL SRK_DLL_IMPORT
#endif

namespace srk::extensions {
	class SRK_EXT_PNG_CONV_DLL PNGConverter {
	public:
		static constexpr uint32_t HEADER_MAGIC = 0x474E5089;

		static IntrusivePtr<Image> SRK_CALL decode(const ByteArray& source);
		static ByteArray SRK_CALL encode(const Image& img);
	};
}