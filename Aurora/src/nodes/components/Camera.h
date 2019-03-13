#pragma once

#include "nodes/components/AbstractComponent.h"
#include "math/Matrix44.h"

AE_NODE_COMPONENT_NS_BEGIN

class Camera : public AbstractComponent {
public:
	Camera();

	ui32 cullingMask;

	inline const Matrix44& AE_CALL getProjectionMatrix() const;
	void AE_CALL setProjectionMatrix(const Matrix44& pm);

protected:
	Matrix44 _pm;//projectionMatrix
	f32 _aspectRatio;
	f32 _zNear;
	f32 _zFar;
};

AE_NODE_COMPONENT_NS_END

#include "Camera.inl"