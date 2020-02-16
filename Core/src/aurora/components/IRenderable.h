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

namespace aurora::components {
	class AE_DLL IRenderable : public IComponent {
	public:
		IRenderable();

		inline render::IRenderer* AE_CALL getRenderer() const {
			return _renderer;
		}
		inline void AE_CALL setRenderer(render::IRenderer* renderer) {
			_renderer = renderer;
		}

		virtual void AE_CALL collectRenderData(render::IRenderDataCollector& collector) const = 0;

		std::vector<RefPtr<render::RenderPass>> renderPasses;

	protected:
		AE_RTTI_DECLARE_DERIVED(IComponent);

		RefPtr<render::IRenderer> _renderer;
	};
}