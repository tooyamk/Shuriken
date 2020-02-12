#include "IComponent.h"

namespace aurora::components {
	IComponent::IComponent() :
		_enabled(true),
		layer(0xFFFFFFFF),
		_node(nullptr) {
		AE_RTTI_DEFINE();
	}

	void IComponent::setEnabled(bool b) {
		if (_enabled != b) {
			_enabled = b;
			_enabledChanged();
		}
	}

	void IComponent::__setNode(Node* node) {
		auto old = _node;
		_node = node;
		_nodeChanged(old);
	}
}