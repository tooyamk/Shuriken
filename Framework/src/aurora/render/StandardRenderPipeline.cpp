#include "StandardRenderPipeline.h"
#include "aurora/Node.h"
#include "aurora/ShaderPredefine.h"
#include "aurora/components/Camera.h"
#include "aurora/components/lights/ILight.h"
#include "aurora/components/renderables/IRenderable.h"
#include "aurora/render/IRenderer.h"
#include <algorithm>

namespace aurora::render {
	StandardRenderPipeline::RenderDataCollector::RenderDataCollector(StandardRenderPipeline& pipeline) :
		_pipeline(pipeline) {
	}

	void StandardRenderPipeline::RenderDataCollector::commit() {
		_pipeline._appendRenderData(*this);
	}


	StandardRenderPipeline::RenderCollector::RenderCollector(StandardRenderPipeline& pipeline) :
		_pipeline(pipeline) {
	}

	void StandardRenderPipeline::RenderCollector::addCamera(components::Camera* camera) {
		_pipeline._addCamera(camera);
	}

	void StandardRenderPipeline::RenderCollector::addRenderable(components::renderables::IRenderable* renderable) {
		_pipeline._addRenderable(renderable);
	}

	void StandardRenderPipeline::RenderCollector::addLight(components::lights::ILight* light) {
		_pipeline._addLight(light);
	}


	StandardRenderPipeline::StandardRenderPipeline() :
		_renderDataPoolVernier(0),
		_shaderParameters(new ShaderParameterCollection()),
		_shaderDefineStack(new ShaderDefineGetterStack()),
		_shaderParameterStack(new ShaderParameterGetterStack()) {
		_builtinShaderParameters.m34_w2v = new ShaderParameter();
		_builtinShaderParameters.m44_w2p = new ShaderParameter();
		_builtinShaderParameters.v3_camPos = new ShaderParameter();
		_shaderParameters->set(ShaderPredefine::MATRIX_WV, _builtinShaderParameters.m34_w2v);
		_shaderParameters->set(ShaderPredefine::MATRIX_WP, _builtinShaderParameters.m44_w2p);
		_shaderParameters->set(ShaderPredefine::CAMERA_POS, _builtinShaderParameters.v3_camPos);

		_shaderParameters->set(ShaderPredefine::AMBIENT_COLOR, new ShaderParameter())->set(Vec3f32::ZERO);
		_shaderParameters->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
		_shaderParameters->set(ShaderPredefine::SPECULAR_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
	}

	void StandardRenderPipeline::render(modules::graphics::IGraphicsModule* graphics, const std::function<void(IRenderCollector&)>& fn) {
		if (!graphics || !fn) return;

		{
			RenderCollector rc(*this);
			fn(rc);
		}

		for (auto& cam : _cameras) {
			auto camLayer = cam->layer;

			RenderDataCollector collector(*this);
			collector.matrix.w2v = cam->getNode()->getInverseWorldMatrix();
			collector.matrix.w2v.append(cam->getProjectionMatrix(), collector.matrix.w2p);

			for (auto& r : _renderables) {
				if ((camLayer & r->layer) == 0) continue;

				collector.data.renderable = r;
				r->getRenderer()->collectRenderData(collector);
			}

			if (!_renderQueue.empty()) {
				_builtinShaderParameters.m34_w2v->set(collector.matrix.w2v);
				_builtinShaderParameters.m44_w2p->set(collector.matrix.w2p);
				_builtinShaderParameters.v3_camPos->set(cam->getNode()->getWorldPosition());

				std::stable_sort(_renderQueue.begin(), _renderQueue.end(), [](const RenderData* lhs, const RenderData* rhs) {
					if (lhs->priority.level1 == rhs->priority.level1) {
						using Lv2_t = std::underlying_type_t<RenderPriority::Level2>;
						if (auto val = (Lv2_t)lhs->priority.level2 - (Lv2_t)rhs->priority.level2; val == 0) {
							switch (lhs->priority.level2) {
							case RenderPriority::Level2::FAR_TO_NEAR:
								return lhs->matrix.l2v[2][3] > rhs->matrix.l2v[2][3];
							case RenderPriority::Level2::NEAR_TO_FAR:
								return lhs->matrix.l2v[2][3] < rhs->matrix.l2v[2][3];
							default:
								return false;
							}
						} else {
							return val < 0;
						}
					} else {
						return lhs->priority.level1 < rhs->priority.level1;
					}
				});
			}

			graphics->beginRender();
			graphics->clear(cam->clearFlag, cam->clearColor, cam->clearDepthValue, cam->clearStencilValue);

			_render();

			_renderEnv.reset();

			graphics->endRender();
		}

		_cameras.clear();
		_renderables.clear();
	}

	void StandardRenderPipeline::_addCamera(components::Camera* camera) {
		if (camera && camera->isEnalbed() && camera->layer && camera->getNode()) _cameras.emplace_back(camera);
	}

	void StandardRenderPipeline::_addRenderable(components::renderables::IRenderable* renderable) {
		if (renderable && renderable->layer && renderable->getRenderer() && renderable->isEnalbed() && renderable->getNode()) _renderables.emplace_back(renderable);
	}

	void StandardRenderPipeline::_addLight(components::lights::ILight* light) {
		if (light && light->layer && light->isEnalbed() && light->getNode()) _renderEnv.lights.emplace_back(light);
	}

	void StandardRenderPipeline::_appendRenderData(RenderDataCollector& collector) {
		if (collector.data.renderer->collectRenderDataConfirm(collector)) {
			RenderData* data;
			if (_renderDataPoolVernier) {
				data = _renderDataPool[--_renderDataPoolVernier];
				data->reset();
			} else {
				data = new RenderData();
				_renderDataPool.emplace_back(data);
			}

			_renderQueue.emplace_back(data);
			data->set(collector.data);

			_renderers.emplace(data->renderer);

			data->matrix.l2w = data->renderable->getNode()->getWorldMatrix();
			data->matrix.l2w.append(collector.matrix.w2v, data->matrix.l2v);
			data->matrix.l2w.append(collector.matrix.w2p, data->matrix.l2p);
		}
	}

	void StandardRenderPipeline::_render() {
		if (auto size = _renderQueue.size(); size) {
			for (auto& r : _renderers) r->preRender(_renderEnv);

			_shaderParameterStack->push(*_shaderParameters);

			auto renderer = _renderQueue[0]->renderer;
			size_t begin = 0;
			for (size_t i = 1; i < size; ++i) {
				auto& data = _renderQueue[i];
				if (data->renderer != renderer) {
					if (renderer) renderer->render(_renderQueue.data() + begin, i - begin, *_shaderDefineStack, *_shaderParameterStack);
					renderer = data->renderer;
					begin = i;
				}
			}

			if (renderer) renderer->render(_renderQueue.data() + begin, size - begin, *_shaderDefineStack, *_shaderParameterStack);

			_renderQueue.clear();
			_renderDataPoolVernier = _renderDataPool.size();

			_shaderParameterStack->pop();

			for (auto& r : _renderers) r->postRender();
			_renderers.clear();
		}
	}
}