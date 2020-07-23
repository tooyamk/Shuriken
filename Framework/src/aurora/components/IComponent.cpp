#include "IComponent.h"

namespace aurora::components {
	IComponent::~IComponent() {
	}

	IComponent::IComponent() :
		_enabled(true),
		layer((std::numeric_limits<decltype(layer)>::max)()),
		_node(nullptr) {
		AE_RTTI_DEFINE();
	}
}