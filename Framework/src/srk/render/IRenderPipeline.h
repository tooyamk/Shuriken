#pragma once

#include "srk/SceneNode.h"

namespace srk::components {
	class Camera;

	namespace lights {
		class ILight;
	}
}

namespace srk::modules::graphics {
	class IGraphicsModule;
}

namespace srk::render {
	class IRenderCollector;

	class SRK_FW_DLL IRenderPipeline : public Ref {
	public:
		virtual ~IRenderPipeline() {}

		virtual void SRK_CALL render(modules::graphics::IGraphicsModule* graphics, const std::function<void(IRenderCollector&)>& fn) = 0;
	};
}