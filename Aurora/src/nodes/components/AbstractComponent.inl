namespace aurora::nodes::component {
	inline bool AbstractComponent::getEnalbed() const {
		return _enabled;
	}

	inline Node* AbstractComponent::getNode() const {
		return _node;
	}
}