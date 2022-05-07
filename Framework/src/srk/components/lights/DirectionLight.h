#pragma once

#include "srk/components/lights/ILight.h"

namespace srk::components::lights {
	class SRK_FW_DLL DirectionLight : public SRK_COMPONENT_INHERIT(ILight)
	public:
		DirectionLight();
	};
}