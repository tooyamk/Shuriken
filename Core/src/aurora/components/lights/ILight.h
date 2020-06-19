#pragma once

#include "aurora/components/IComponent.h"
#include "aurora/math/Vector.h"

namespace aurora::components::lights {
	class AE_DLL ILight : public AE_COMPONENT_INHERIT(IComponent)
	public:
		ILight();

		inline const Vec3f32& AE_CALL getColor() const {
			return _color;
		}

		inline void AE_CALL setColor(const Vec3f32& color) {
			_color = color;
		}

		inline float32_t AE_CALL getIntensity() const {
			return _intensity;
		}

		inline void AE_CALL setIntensity(float32_t intensity) {
			_intensity = intensity;
		}

	protected:
		Vec3f32 _color;
		float32_t _intensity;
	};
}