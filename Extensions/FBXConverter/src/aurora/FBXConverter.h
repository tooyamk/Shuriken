#pragma once

#include "aurora/Mesh.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL FBXConverter {
	public:
		struct AE_EXTENSION_DLL Result {
			std::vector<IntrusivePtr<MeshResource>> meshes;
		};

		static Result AE_CALL decode(const ByteArray& source);
	};
}