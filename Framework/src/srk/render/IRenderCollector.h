#pragma once

#include "srk/Global.h"

namespace srk::components {
	class Camera;

	namespace renderables {
		class IRenderable;
	}

	namespace lights {
		class ILight;
	}
}

namespace srk::render {


	class SRK_FW_DLL IRenderCollector {
	public:
		virtual ~IRenderCollector() {}

		virtual void SRK_CALL addCamera(components::Camera*) = 0;
		virtual void SRK_CALL addRenderable(components::renderables::IRenderable*) = 0;
		virtual void SRK_CALL addLight(components::lights::ILight*) = 0;
	};
}