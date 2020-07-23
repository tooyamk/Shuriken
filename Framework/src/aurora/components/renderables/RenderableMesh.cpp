#include "RenderableMesh.h"
#include "aurora/render/IRenderDataCollector.h"

namespace aurora::components::renderables {
	RenderableMesh::RenderableMesh() {
		AE_RTTI_DEFINE();
	}

	void RenderableMesh::collectRenderData(render::IRenderDataCollector& collector) const {
		if (_mesh && !_mesh->getVertexBuffers().isEmpty() && _mesh->getIndexBuffer()) {
			collector.data.meshGetter.set(&RenderableMesh::_meshGetter, (void*)this);

			collector.commit();

			collector.data.meshGetter = nullptr;
		}
	}

	const Mesh* RenderableMesh::_meshGetter(void* target) {
		return ((RenderableMesh*)target)->_mesh;
	}
}