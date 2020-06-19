#pragma once

#include "aurora/Node.h"

namespace aurora::components {
	class Camera;

	namespace lights {
		class ILight;
	}
}

namespace aurora::modules::graphics {
	class IGraphicsModule;
}

namespace aurora::render {
	class IRenderCollector;

	class AE_DLL IRenderPipeline : public Ref {
	public:
		virtual ~IRenderPipeline() {}

		virtual void AE_CALL render(modules::graphics::IGraphicsModule* graphics, const std::function<void(IRenderCollector&)>& fn) = 0;
	};
}