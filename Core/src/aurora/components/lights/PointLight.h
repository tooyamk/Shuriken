#pragma once

#include "aurora/components/lights/ILight.h"

namespace aurora::components::lights {
	class AE_DLL PointLight : public ILight {
	public:
		PointLight();

		inline f32 AE_CALL getRadius() const {
			return _radius;
		}

		inline void AE_CALL setRadius(f32 radius) {
			_radius = radius;
		}

	protected:
		AE_RTTI_DECLARE_DERIVED(ILight);

		f32 _radius;
	};
}