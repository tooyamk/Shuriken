#pragma once

#include "aurora/Global.h"

namespace aurora::components {
	class Camera;

	namespace renderables {
		class IRenderable;
	}

	namespace lights {
		class ILight;
	}
}

namespace aurora::render {


	class AE_FW_DLL IRenderCollector {
	public:
		virtual ~IRenderCollector() {}

		virtual void AE_CALL addCamera(components::Camera*) = 0;
		virtual void AE_CALL addRenderable(components::renderables::IRenderable*) = 0;
		virtual void AE_CALL addLight(components::lights::ILight*) = 0;
	};
}