#include "AbstractComponent.h"

AE_NODE_COMPONENT_NS_BEGIN

AbstractComponent::AbstractComponent() :
	_enabled(true),
	flag(0),
	layer(0xFFFFFFFF),
	_node(nullptr) {
}

void AbstractComponent::setEnabled(bool b) {
	if (_enabled != b) {
		_enabled = b;
		_enabledChanged();
	}
}

void AbstractComponent::__setNode(Node* node) {
	auto old = _node;
	_node = node;
	_nodeChanged(old);
}

AE_NODE_COMPONENT_NS_END