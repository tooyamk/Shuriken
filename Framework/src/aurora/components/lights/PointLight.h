#pragma once

#include "aurora/components/lights/ILight.h"

namespace aurora::components::lights {
	class AE_FW_DLL PointLight : public AE_COMPONENT_INHERIT(ILight)
	public:
		PointLight();

		inline float32_t AE_CALL getRadius() const {
			return _radius;
		}

		inline void AE_CALL setRadius(float32_t radius) {
			_radius = radius;
		}

	protected:
		float32_t _radius;
	};
}