#pragma once

#include "srk/components/IComponent.h"
#include "srk/render/IRenderer.h"
#include "srk/render/RenderPass.h"
#include <vector>

namespace srk::render {
	class IRenderer;
	class IRenderDataCollector;
	class RenderPass;
}

namespace srk::components::renderables {
	class SRK_FW_DLL IRenderable : public SRK_COMPONENT_INHERIT(IComponent)
	public:
		IRenderable();

		inline render::IRenderer* SRK_CALL getRenderer() const {
			return _renderer;
		}
		inline void SRK_CALL setRenderer(render::IRenderer* renderer) {
			_renderer = renderer;
		}

		virtual void SRK_CALL collectRenderData(render::IRenderDataCollector& collector) const = 0;

		std::vector<IntrusivePtr<render::RenderPass>> renderPasses;

	protected:
		IntrusivePtr<render::IRenderer> _renderer;
	};
}