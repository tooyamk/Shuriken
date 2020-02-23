#pragma once

#include "aurora/components/IComponent.h"
#include "aurora/math/Matrix.h"
#include "aurora/math/Vector.h"

namespace aurora::modules::graphics {
	enum class ClearFlag : uint8_t;
}

namespace aurora::components {
	class AE_DLL Camera : public IComponent {
	public:
		Camera();

		uint32_t cullingMask;

		modules::graphics::ClearFlag clearFlag;
		Vec4f32 clearColor;
		f32 clearDepthValue;
		size_t clearStencilValue;

		inline f32 AE_CALL getAspectRatio() const {
			return _aspectRatio;
		}

		inline f32 AE_CALL getNearClipPlane() const {
			return _zNear;
		}

		inline f32 AE_CALL getFarClipPlane() const {
			return _zFar;
		}

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