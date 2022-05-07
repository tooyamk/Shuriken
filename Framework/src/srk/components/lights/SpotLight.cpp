#include "SpotLight.h"
#include "srk/math/Math.h"

namespace srk::components::lights {
	SpotLight::SpotLight() :
		_radius(1000.0f),
		_spotAngle(Math::PI_6<decltype(_spotAngle)>) {
		SRK_RTTI_DEFINE();
	}
}