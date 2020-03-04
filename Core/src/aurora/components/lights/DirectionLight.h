#pragma once

#include "aurora/components/lights/ILight.h"

namespace aurora::components::lights {
	class AE_DLL DirectionLight : public ILight {
	public:
		DirectionLight();

	protected:
		AE_RTTI_DECLARE_DERIVED(ILight);
	};
}