#include "IComponent.h"

namespace srk::components {
	IComponent::~IComponent() {
	}

	IComponent::IComponent() :
		_enabled(true),
		layer((std::numeric_limits<decltype(layer)>::max)()),
		_node(nullptr) {
		SRK_RTTI_DEFINE();
	}
}