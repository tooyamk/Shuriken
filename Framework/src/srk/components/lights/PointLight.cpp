#include "PointLight.h"

namespace srk::components::lights {
	PointLight::PointLight() :
		_radius(1000.0f) {
		SRK_RTTI_DEFINE();
	}
}