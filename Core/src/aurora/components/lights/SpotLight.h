#pragma once

#include "aurora/components/lights/ILight.h"

namespace aurora::components::lights {
	class AE_DLL SpotLight : public ILight {
	public:
		SpotLight();

		inline f32 AE_CALL getRadius() const {
			return _radius;
		}

		inline void AE_CALL setRadius(f32 radius) {
			_radius = radius;
		}

		inline f32 AE_CALL getSpotAngle() const {
			return _spotAngle;
		}

		inline void AE_CALL setSpotAngle(f32 angle) {
			_spotAngle = angle;
		}

	protected:
		AE_RTTI_DECLARE_DERIVED(ILight);

		f32 _radius;
		f32 _spotAngle;
	};
}