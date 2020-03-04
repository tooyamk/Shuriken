#include "PointLight.h"

namespace aurora::components::lights {
	PointLight::PointLight() :
		_radius(1000.0f) {
		AE_RTTI_DEFINE();
	}
}