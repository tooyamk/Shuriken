#pragma once

#include "aurora/components/lights/ILight.h"

namespace aurora::components::lights {
	class AE_FW_DLL SpotLight : public AE_COMPONENT_INHERIT(ILight)
	public:
		SpotLight();

		inline float32_t AE_CALL getRadius() const {
			return _radius;
		}

		inline void AE_CALL setRadius(float32_t radius) {
			_radius = radius;
		}

		inline float32_t AE_CALL getSpotAngle() const {
			return _spotAngle;
		}

		inline void AE_CALL setSpotAngle(float32_t angle) {
			_spotAngle = angle;
		}

	protected:
		float32_t _radius;
		float32_t _spotAngle;
	};
}