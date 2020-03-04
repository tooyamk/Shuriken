#pragma once

#include "aurora/components/renderables/IRenderable.h"
#include "aurora/Mesh.h"

namespace aurora::components::renderables {
	class AE_DLL RenderableMesh : public IRenderable {
	public:
		RenderableMesh();

		virtual void AE_CALL collectRenderData(render::IRenderDataCollector& collector) const override;

		inline Mesh* AE_CALL getMesh() const {
			return _mesh;
		}

		inline void AE_CALL setMesh(Mesh* mesh) {
			_mesh = mesh;
		}

	protected:
		AE_RTTI_DECLARE_DERIVED(IRenderable);

		RefPtr<Mesh> _mesh;
	};
}