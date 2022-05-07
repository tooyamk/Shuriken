#pragma once

#include "srk/components/IComponent.h"
#include "srk/math/Vector.h"

namespace srk::components::lights {
	class SRK_FW_DLL ILight : public SRK_COMPONENT_INHERIT(IComponent)
	public:
		ILight();

		inline const Vec3f32& SRK_CALL getColor() const {
			return _color;
		}

		inline void SRK_CALL setColor(const Vec3f32& color) {
			_color = color;
		}

		inline float32_t SRK_CALL getIntensity() const {
			return _intensity;
		}

		inline void SRK_CALL setIntensity(float32_t intensity) {
			_intensity = intensity;
		}

	protected:
		Vec3f32 _color;
		float32_t _intensity;
	};
}