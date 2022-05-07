#pragma once

#include "srk/components/renderables/IRenderable.h"
#include "srk/Mesh.h"

namespace srk::components::renderables {
	class SRK_FW_DLL RenderableMesh : public SRK_COMPONENT_INHERIT(IRenderable)
	public:
		RenderableMesh();

		virtual void SRK_CALL collectRenderData(render::IRenderDataCollector& collector) const override;

		inline Mesh* SRK_CALL getMesh() const {
			return _mesh;
		}

		inline void SRK_CALL setMesh(Mesh* mesh) {
			_mesh = mesh;
		}

	protected:
		IntrusivePtr<Mesh> _mesh;

		static const Mesh* SRK_CALL _meshGetter(void* target);
	};
}