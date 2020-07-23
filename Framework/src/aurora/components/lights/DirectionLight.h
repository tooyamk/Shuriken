#pragma once

#include "aurora/components/lights/ILight.h"

namespace aurora::components::lights {
	class AE_FW_DLL DirectionLight : public AE_COMPONENT_INHERIT(ILight)
	public:
		DirectionLight();
	};
}