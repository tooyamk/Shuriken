#include "Material.h"

namespace aurora {
	Material::Material() {
		_defines.ref();
		_parameters.ref();
	}
}