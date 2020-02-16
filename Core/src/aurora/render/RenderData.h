#pragma once

#include "aurora/render/RenderPriority.h"

namespace aurora {
	class Material;
	class Mesh;
}

namespace aurora::components {
	class IRenderable;
}

namespace aurora::render {
	class IRenderer;
	class RenderState;


	class AE_DLL RenderData {
	public:
		RenderData() {
			reset();
		}

		const components::IRenderable* renderable;
		RenderPriority priority;
		RenderState* state;
		Material* material;
		const Mesh* mesh;
		const IRenderer* renderer;

		inline void AE_CALL set(const RenderData& data) {
			renderable = data.renderable;
			state = data.state;
			priority = data.priority;
			material = data.material;
			mesh = data.mesh;
			renderer = data.renderer;
		}

		inline void AE_CALL reset() {
			renderable = nullptr;
			state = nullptr;
			priority.reset();
			material = nullptr;
			mesh = nullptr;
			renderer = nullptr;
		}
	};
}