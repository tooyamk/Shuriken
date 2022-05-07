#pragma once

#include "srk/Mesh.h"

namespace srk::extensions {
	class SRK_EXTENSION_DLL FBXConverter {
	public:
		struct SRK_EXTENSION_DLL Result {
			std::vector<IntrusivePtr<MeshResource>> meshes;
		};

		static Result SRK_CALL decode(const ByteArray& source);
	};
}