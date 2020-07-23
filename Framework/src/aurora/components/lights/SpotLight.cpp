#include "SpotLight.h"
#include "aurora/math/Math.h"

namespace aurora::components::lights {
	SpotLight::SpotLight() :
		_radius(1000.0f),
		_spotAngle(Math::PI_6<decltype(_spotAngle)>) {
		AE_RTTI_DEFINE();
	}
}