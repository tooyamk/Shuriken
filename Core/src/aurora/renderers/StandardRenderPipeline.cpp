#include "StandardRenderPipeline.h"
#include "aurora/Node.h"
#include "aurora/components/IRenderable.h"

namespace aurora::renderers {
	void StandardRenderPipeline::render(Node* node) {
		if (node) {
			_collectNode(node);
		}
	}

	void StandardRenderPipeline::_collectNode(Node* node) {
		node->getComponents<components::IRenderable>(_renderables);

		for (auto& child : *node) _collectNode(child);
	}
}