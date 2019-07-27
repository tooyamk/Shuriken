#pragma once

#include "nodes/components/AbstractComponent.h"
#include "math/Matrix44.h"

namespace aurora::nodes::component {
	class Camera : public AbstractComponent {
	public:
		Camera();

		uint32_t cullingMask;

		inline const Matrix44& AE_CALL getProjectionMatrix() const;
		void AE_CALL setProjectionMatrix(const Matrix44& pm);

	protected:
		Matrix44 _pm;//projectionMatrix
		f32 _aspectRatio;
		f32 _zNear;
		f32 _zFar;
	};
}

#include "Camera.inl"