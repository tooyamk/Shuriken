#pragma once

#include "srk/components/IComponent.h"
#include "srk/math/Matrix.h"
#include "srk/math/Vector.h"

namespace srk::modules::graphics {
	enum class ClearFlag : uint8_t;
}

namespace srk::components {
	class SRK_FW_DLL Camera : public SRK_COMPONENT_INHERIT(IComponent)
	public:
		Camera();

		uint32_t cullingMask;

		modules::graphics::ClearFlag clearFlag;
		Vec4f32 clearColor;
		float32_t clearDepthValue;
		size_t clearStencilValue;

		inline float32_t SRK_CALL getAspectRatio() const {
			return _aspectRatio;
		}

		inline float32_t SRK_CALL getNearClipPlane() const {
			return _zNear;
		}

		inline float32_t SRK_CALL getFarClipPlane() const {
			return _zFar;
		}

		inline const Matrix44& SRK_CALL getProjectionMatrix() const {
			return _pm;
		}
		void SRK_CALL setProjectionMatrix(const Matrix44& pm);

	protected:
		Matrix44 _pm;//projectionMatrix

		float32_t _aspectRatio;
		float32_t _zNear;
		float32_t _zFar;
	};
}