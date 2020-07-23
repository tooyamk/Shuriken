#include "ILight.h"

namespace aurora::components::lights {
	ILight::ILight() :
		_color(1.0f),
		_intensity(1.0f) {
		AE_RTTI_DEFINE();
	}
}