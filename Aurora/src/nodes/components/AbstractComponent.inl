AE_NODE_COMPONENT_NS_BEGIN

inline bool AbstractComponent::getEnalbed() const {
	return _enabled;
}

inline Node* AbstractComponent::getNode() const {
	return _node;
}

AE_NODE_COMPONENT_NS_END