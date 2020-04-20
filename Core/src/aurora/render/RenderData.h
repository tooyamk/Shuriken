#pragma once

#include "aurora/math/Matrix.h"
#include "aurora/render/RenderPass.h"
#include "aurora/render/RenderPriority.h"

namespace aurora {
	class Material;
	class Mesh;
}

namespace aurora::components::renderables {
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

		const components::renderables::IRenderable* renderable;
		RenderPriority priority;
		RenderState* state;
		Material* material;
		const Mesh* mesh;
		IRenderer* renderer;
		std::vector<RefPtr<RenderPass>>* subPasses;

		struct {
			Matrix34 l2w;
			Matrix34 l2v;
			Matrix44 l2p;
		} matrix;

		inline void AE_CALL set(const RenderData& data) {
			renderable = data.renderable;
			state = data.state;
			priority = data.priority;
			material = data.material;
			mesh = data.mesh;
			renderer = data.renderer;
			subPasses = data.subPasses;
		}

		inline void AE_CALL reset() {
			renderable = nullptr;
			state = nullptr;
			priority.reset();
			material = nullptr;
			mesh = nullptr;
			renderer = nullptr;
			subPasses = nullptr;
		}
	};
}