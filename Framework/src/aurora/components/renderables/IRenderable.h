#pragma once

#include "aurora/components/IComponent.h"
#include "aurora/render/IRenderer.h"
#include "aurora/render/RenderPass.h"
#include <vector>

namespace aurora::render {
	class IRenderer;
	class IRenderDataCollector;
	class RenderPass;
}

namespace aurora::components::renderables {
	class AE_FW_DLL IRenderable : public AE_COMPONENT_INHERIT(IComponent)
	public:
		IRenderable();

		inline render::IRenderer* AE_CALL getRenderer() const {
			return _renderer;
		}
		inline void AE_CALL setRenderer(render::IRenderer* renderer) {
			_renderer = renderer;
		}

		virtual void AE_CALL collectRenderData(render::IRenderDataCollector& collector) const = 0;

		std::vector<IntrusivePtr<render::RenderPass>> renderPasses;

	protected:
		IntrusivePtr<render::IRenderer> _renderer;
	};
}