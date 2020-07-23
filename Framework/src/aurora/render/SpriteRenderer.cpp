#include "SpriteRenderer.h"
#include "aurora/components/renderables/IRenderable.h"
#include "aurora/render/IRenderDataCollector.h"

namespace aurora::render {
	SpriteRenderer::SpriteRenderer(modules::graphics::IGraphicsModule& graphics) {

	}

	void SpriteRenderer::collectRenderData(IRenderDataCollector& collector) {
		auto& data = collector.data;
		data.renderer = this;

		auto renderable = data.renderable;
		for (auto& pass : renderable->renderPasses) {
			/*
			if (pass && pass->material && pass->tags && pass->tags->has(_baseTag)) {
				data.priority = pass->priority;
				data.state = pass->state;
				data.material = pass->material;
				data.subPasses = &pass->subPasses;

				renderable->collectRenderData(collector);

				data.priority.reset();
				data.state = nullptr;
				data.material = nullptr;
				data.subPasses = nullptr;
			}
			*/
		}

		data.renderer = nullptr;
	}

	bool SpriteRenderer::collectRenderDataConfirm(IRenderDataCollector& collector) const {
		return collector.data.meshGetter;
	}

	void SpriteRenderer::preRender(const RenderEnvironment& env) {
	}

	void SpriteRenderer::render(RenderData* const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) {
	}

	void SpriteRenderer::postRender() {
	}
}