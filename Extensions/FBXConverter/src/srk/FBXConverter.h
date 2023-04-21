#pragma once

#include "srk/Mesh.h"

#ifdef SRK_EXT_FBX_CONV_EXPORTS
#	define SRK_EXT_FBX_CONV_DLL SRK_DLL_EXPORT
#else
#	define SRK_EXT_FBX_CONV_DLL SRK_DLL_IMPORT
#endif

namespace srk::extensions {
	class SRK_EXT_FBX_CONV_DLL FBXConverter {
	public:
		struct SRK_EXT_FBX_CONV_DLL Result {
			std::vector<IntrusivePtr<MeshResource>> meshes;
		};

		static Result SRK_CALL decode(const ByteArray& source);
	};
}