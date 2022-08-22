#include "Camera.h"
#include "srk/modules/graphics/IGraphicsModule.h"

namespace srk::components {
	Camera::Camera() :
		cullingMask(0xFFFFFFFF),
		clearFlag(modules::graphics::ClearFlag::ALL),
		clearColor(0.f, 0.f, 0.f, 1.f),
		clearDepthValue(1.0f),
		clearStencilValue(0),
		_aspectRatio(0.f),
		_zNear(0.f),
		_zFar(0.f) {
		SRK_RTTI_DEFINE();
	}

	void Camera::setProjectionMatrix(const Matrix4x4f32& pm) {
		_pm.set(pm);
		auto& m = _pm.data;

		_zNear = -m[2][3] / m[2][2];
		_zFar = (m[2][3] - m[3][3]) / (m[3][2] - m[2][2]);

		_aspectRatio = m[1][1] / m[0][0];
	}
}