#include "RenderableMesh.h"
#include "srk/render/IRenderDataCollector.h"

namespace srk::components::renderables {
	RenderableMesh::RenderableMesh() {
		SRK_RTTI_DEFINE();
	}

	void RenderableMesh::collectRenderData(render::IRenderDataCollector& collector) const {
		if (_mesh) {
			if (auto buffer = _mesh->getBuffer(); buffer && !buffer->getVertices().isEmpty()) {
				collector.data.mesh.set(&RenderableMesh::_meshGetter, (void*)this);

				collector.commit();

				collector.data.mesh = nullptr;
			}
		}
	}

	const Mesh* RenderableMesh::_meshGetter(void* target) {
		return ((RenderableMesh*)target)->_mesh;
	}
}