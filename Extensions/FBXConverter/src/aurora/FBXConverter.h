#pragma once

#include "aurora/MeshResource.h"

namespace aurora::extensions {
	class AE_EXTENSION_DLL FBXConverter {
	public:
		AE_DECLARE_CANNOT_INSTANTIATE(FBXConverter);

		struct AE_EXTENSION_DLL Result {
			std::vector<RefPtr<MeshResource>> meshes;
		};

		static Result AE_CALL parse(const ByteArray& source);
	};
}