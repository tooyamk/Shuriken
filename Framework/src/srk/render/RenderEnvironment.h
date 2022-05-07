#pragma once

#include "srk/Global.h"
#include <vector>

namespace srk::components::lights {
	class ILight;
}

namespace srk::render {
	class SRK_FW_DLL RenderEnvironment {
	public:
		std::vector<components::lights::ILight*> lights;

		inline void SRK_CALL reset() {
			lights.clear();
		}
	};
}