#include "RenderableMesh.h"
#include "aurora/render/IRenderDataCollector.h"

namespace aurora::components {
	RenderableMesh::RenderableMesh() {
		AE_RTTI_DEFINE();
	}

	void RenderableMesh::collectRenderData(render::IRenderDataCollector& collector) const {
		collector.data.mesh = _mesh;

		collector.commit();

		collector.data.mesh = nullptr;
	}
}