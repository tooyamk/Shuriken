#include "ILight.h"

namespace srk::components::lights {
	ILight::ILight() :
		_color(1.0f),
		_intensity(1.0f) {
		SRK_RTTI_DEFINE();
	}
}