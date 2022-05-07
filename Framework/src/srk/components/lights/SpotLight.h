#pragma once

#include "srk/components/lights/ILight.h"

namespace srk::components::lights {
	class SRK_FW_DLL SpotLight : public SRK_COMPONENT_INHERIT(ILight)
	public:
		SpotLight();

		inline float32_t SRK_CALL getRadius() const {
			return _radius;
		}

		inline void SRK_CALL setRadius(float32_t radius) {
			_radius = radius;
		}

		inline float32_t SRK_CALL getSpotAngle() const {
			return _spotAngle;
		}

		inline void SRK_CALL setSpotAngle(float32_t angle) {
			_spotAngle = angle;
		}

	protected:
		float32_t _radius;
		float32_t _spotAngle;
	};
}