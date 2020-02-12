#pragma once

#include "aurora/components/IComponent.h"
#include "aurora/math/Matrix.h"

namespace aurora::components {
	class Camera : public IComponent {
	public:
		Camera();

		uint32_t cullingMask;

		inline const Matrix44& AE_CALL getProjectionMatrix() const {
			return _pm;
		}
		void AE_CALL setProjectionMatrix(const Matrix44& pm);

	protected:
		Matrix44 _pm;//projectionMatrix
		f32 _aspectRatio;
		f32 _zNear;
		f32 _zFar;

		AE_RTTI_DECLARE_DERIVED(IComponent);
	};
}