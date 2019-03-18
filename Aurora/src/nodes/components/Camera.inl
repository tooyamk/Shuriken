namespace aurora::node::component {
	inline const Matrix44& Camera::getProjectionMatrix() const {
		return _pm;
	}
}