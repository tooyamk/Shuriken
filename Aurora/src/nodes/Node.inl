namespace aurora::nodes {
	inline Node* Node::getRoot() const {
		return _root;
	}

	inline Node* Node::getParent() const {
		return _parent;
	}

	inline ui32 Node::getNumChildren() const {
		return _numChildren;
	}

	inline void Node::getLocalPosition(f32(&dst)[3]) const {
		auto x = _lm.data[0][3], y = _lm.data[1][3], z = _lm.data[2][3];
		dst[0] = x;
		dst[1] = y;
		dst[2] = z;
	}

	inline const Quaternion& Node::getLocalRotation() const {
		return _lr;
	}

	inline const Vec3f32& Node::getLocalScale() const {
		return _ls;
	}

	inline const Matrix34& Node::getLocalMatrix() const {
		return _lm;
	}

	inline const Quaternion& Node::getWorldRotation() const {
		updateWorldRotation();
		return _wr;
	}

	inline void Node::getWorldPosition(f32(&dst)[3]) const {
		updateWorldMatrix();

		auto x = _wm.data[0][3], y = _wm.data[1][3], z = _wm.data[2][3];
		dst[0] = x;
		dst[1] = y;
		dst[2] = z;
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
		_lm.decomposition(&rot, _ls);
		rot.toQuaternion(_lr);
	}

	inline void Node::_checkNoticeUpdate(ui32 dirty) {
		_checkNoticeUpdateNow(_dirty | dirty, dirty);
	}

	inline void Node::_checkNoticeUpdate(ui32 appendDirty, ui32 sendDirty) {
		_checkNoticeUpdateNow(_dirty | appendDirty, sendDirty);
	}
}