#pragma once

#include "aurora/Node.h"

namespace aurora::components {
	class Camera;
}

namespace aurora::modules::graphics {
	class IGraphicsModule;
}

namespace aurora::render {
	class AE_DLL IRenderPipeline : public Ref {
	public:
		virtual ~IRenderPipeline() {}

		virtual void AE_CALL render(modules::graphics::IGraphicsModule* graphics, components::Camera* camera, Node* node) = 0;
	};
}