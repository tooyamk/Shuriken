#pragma once

#include "srk/components/lights/ILight.h"

namespace srk::components::lights {
	class SRK_FW_DLL PointLight : public SRK_COMPONENT_INHERIT(ILight)
	public:
		PointLight();

		inline float32_t SRK_CALL getRadius() const {
			return _radius;
		}

		inline void SRK_CALL setRadius(float32_t radius) {
			_radius = radius;
		}

	protected:
		float32_t _radius;
	};
}