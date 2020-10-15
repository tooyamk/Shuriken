#include "SpriteRenderer.h"
#include "aurora/Mesh.h"
#include "aurora/StackPopper.h"
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
		return collector.data.mesh;
	}

	void SpriteRenderer::preRender(const RenderEnvironment& env) {
	}

	void SpriteRenderer::render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) {
		Material* material = nullptr;

		for (size_t i = 0; i < count; ++i) {
			auto rd = data[i];

			auto mat = rd->material;
			if (!mat) continue;

			if (auto mesh = rd->mesh(); mesh) {
				if (auto meshResource = mesh->getResource(); meshResource) {
					if (mat != material) {
						_flush(material);
						material = mat;

						auto shader = material->getShader();
						if (!shader) {
							material = nullptr;
							continue;
						}

						{
							StackPopper<ShaderDefineGetterStack, StackPopperFlag::MULTI_POP> popper(shaderDefineStack, shaderDefineStack.push(material->getDefines()));

							auto program = shader->select(&shaderDefineStack);
							if (program) {
								//todo
							} else {
								material = nullptr;
								continue;
							}
						}
					}
				}
			}
		}

		_flush(material);
	}

	void SpriteRenderer::postRender() {
	}

	void SpriteRenderer::_flush(Material* matrial) {
		if (matrial) {

		}
	}
}