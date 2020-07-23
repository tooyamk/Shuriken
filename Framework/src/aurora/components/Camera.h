#pragma once

#include "aurora/components/IComponent.h"
#include "aurora/math/Matrix.h"
#include "aurora/math/Vector.h"

namespace aurora::modules::graphics {
	enum class ClearFlag : uint8_t;
}

namespace aurora::components {
	class AE_FW_DLL Camera : public AE_COMPONENT_INHERIT(IComponent)
	public:
		Camera();

		uint32_t cullingMask;

		modules::graphics::ClearFlag clearFlag;
		Vec4f32 clearColor;
		float32_t clearDepthValue;
		size_t clearStencilValue;

		inline float32_t AE_CALL getAspectRatio() const {
			return _aspectRatio;
		}

		inline float32_t AE_CALL getNearClipPlane() const {
			return _zNear;
		}

		inline float32_t AE_CALL getFarClipPlane() const {
			return _zFar;
		}

		inline const Matrix44& AE_CALL getProjectionMatrix() const {
			return _pm;
		}
		void AE_CALL setProjectionMatrix(const Matrix44& pm);

	protected:
		Matrix44 _pm;//projectionMatrix

		float32_t _aspectRatio;
		float32_t _zNear;
		float32_t _zFar;
	};
}