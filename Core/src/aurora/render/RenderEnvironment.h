#pragma once

#include "aurora/Global.h"
#include <vector>

namespace aurora::components::lights {
	class ILight;
}

namespace aurora::render {
	class AE_DLL RenderEnvironment {
	public:
		std::vector<components::lights::ILight*> lights;

		inline void AE_CALL reset() {
			lights.clear();
		}
	};
}