#include "StandardRenderPipeline.h"
#include "aurora/Node.h"
#include "aurora/components/IRenderable.h"
#include "aurora/render/IRenderer.h"
#include <algorithm>

namespace aurora::render {
	StandardRenderPipeline::RenderDataCollector::RenderDataCollector(StandardRenderPipeline& pipeline) :
		_pipeline(pipeline) {
	}

	void StandardRenderPipeline::RenderDataCollector::commit() {
		_pipeline._appendRenderData(*this);
	}


	StandardRenderPipeline::StandardRenderPipeline() :
		_renderDataPoolVernier(0) {
	}

	void StandardRenderPipeline::render(Node* node) {
		if (node) {
			{
				RenderDataCollector collector(*this);
				_collectNode(node, collector);
			}

			std::stable_sort(_renderQueue.begin(), _renderQueue.end(), [](const RenderData* lhs, const RenderData* rhs) {
				if (lhs->priority.level1 == rhs->priority.level1) {
					using Lv2_t = std::underlying_type<RenderPriority::Level2>::type;
					auto val = (Lv2_t)lhs->priority.level2 - (Lv2_t)rhs->priority.level2;
					if (val == 0) {
						//todo
						return false;
					} else {
						return val < 0;
					}
				} else {
					return lhs->priority.level1 < rhs->priority.level1;
				}
			});

			_render();
		}
	}

	void StandardRenderPipeline::_collectNode(Node* node, RenderDataCollector& collector) {
		auto& components = node->getComponents();
		for (auto& c : components) {
			if (c->isKindOf<components::IRenderable>()) {
				auto r = (const components::IRenderable*)c;
				if (r->getRenderer()) {
					collector.data.renderable = r;
					r->getRenderer()->collectRenderData(collector);
				}
			}
		}

		for (auto& child : *node) _collectNode(child, collector);
	}

	void StandardRenderPipeline::_appendRenderData(RenderDataCollector& collector) {
		if (collector.data.renderer->collectRenderDataConfirm(collector)) {
			RenderData* data;
			if (_renderDataPoolVernier) {
				data = _renderDataPool[--_renderDataPoolVernier];
			} else {
				data = new RenderData();
				_renderDataPool.emplace_back(data);
			}

			_renderQueue.emplace_back(data);
			data->set(collector.data);
		}
	}

	void StandardRenderPipeline::_render() {
		if (auto size = _renderQueue.size(); size) {
			auto renderer = _renderQueue[0]->renderer;
			size_t begin = 0;
			for (size_t i = 1; i < size; ++i) {
				auto& data = _renderQueue[i];
				if (data->renderer != renderer) {
					if (renderer) renderer->render(_renderQueue.data() + begin, i - begin, _shaderDefineStack, _shaderParameterStack);
					renderer = data->renderer;
					begin = i;
				}
			}

			if (renderer) renderer->render(_renderQueue.data() + begin, size - begin, _shaderDefineStack, _shaderParameterStack);

			_renderQueue.clear();
			_renderDataPoolVernier = _renderDataPool.size();
		}
	}
}