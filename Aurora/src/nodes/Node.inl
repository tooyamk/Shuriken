namespace aurora::node {
	inline Node* Node::getRoot() const {
		return _root;
	}

	inline Node* Node::getParent() const {
		return _parent;
	}

	inline ui32 Node::getNumChildren() const {
		return _numChildren;
	}

	inline Vector3 Node::getLocalPosition() const {
		return std::move(Vector3(_lm.m34[0][3], _lm.m34[1][3], _lm.m34[2][3]));
	}

	inline void Node::getLocalPosition(Vector3& dst) const {
		dst.set(_lm.m34[0][3], _lm.m34[1][3], _lm.m34[2][3]);
	}

	inline const Quaternion& Node::getLocalRotation() const {
		return _lr;
	}

	inline const Vector3& Node::getLocalScale() const {
		return _ls;
	}

	inline const Matrix34& Node::getLocalMatrix() const {
		return _lm;
	}

	inline const Quaternion& Node::getWorldRotation() const {
		updateWorldRotation();
		return _wr;
	}

	inline Vector3 Node::getWorldPosition() const {
		updateWorldMatrix();
		return std::move(Vector3(_wm.m34[0][3], _wm.m34[1][3], _wm.m34[2][3]));
	}

	inline void Node::getWorldPosition(Vector3& dst) const {
		updateWorldMatrix();
		dst.set(_lm.m34[0][3], _lm.m34[1][3], _lm.m34[2][3]);
	}

	inline const Matrix34& Node::getWorldMatrix() const {
		updateWorldMatrix();
		return _wm;
	}

	inline const Matrix34& Node::getInverseWorldMatrix() const {
		updateInverseWorldMatrix();
		return _iwm;
	}

	inline const std::vector<component::AbstractComponent*>& Node::getComponents() const {
		return _components;
	}

	inline void Node::_localDecomposition() {
		Matrix34 rot;
		_lm.decomposition(&rot, &_ls);
		rot.toQuaternion(_lr);
	}

	inline void Node::_checkNoticeUpdate(ui32 dirty) {
		_checkNoticeUpdateNow(_dirty | dirty, dirty);
	}

	inline void Node::_checkNoticeUpdate(ui32 appendDirty, ui32 sendDirty) {
		_checkNoticeUpdateNow(_dirty | appendDirty, sendDirty);
	}
}