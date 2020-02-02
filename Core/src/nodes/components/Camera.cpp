#include "Camera.h"

namespace aurora::nodes::component {
	Camera::Camera() :
		cullingMask(0xFFFFFFFF),
		_aspectRatio(0.f),
		_zNear(0.f),
		_zFar(0.f) {
		flags |= ComponentFlag::CAMERA;
	}

	void Camera::setProjectionMatrix(const Matrix44& pm) {
		_pm.set44(pm);
		auto& m = _pm.data;

		_zNear = -m[2][3] / m[2][2];

		if (m[3][3] == 1.f) {
			_zFar = 1.f / m[2][2] + _zNear;
		} else {
			_zFar = (_zNear * m[2][2]) / (m[2][2] - 1.f);
		}

		_aspectRatio = m[1][1] / m[0][0];
	}
}