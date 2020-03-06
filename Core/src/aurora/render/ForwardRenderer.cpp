#include "ForwardRenderer.h"
#include "aurora/Mesh.h"
#include "aurora/Node.h"
#include "aurora/ShaderPredefine.h"
#include "aurora/StackPopper.h"
#include "aurora/components/lights/DirectionLight.h"
#include "aurora/components/lights/PointLight.h"
#include "aurora/components/lights/SpotLight.h"
#include "aurora/components/renderables/IRenderable.h"
#include "aurora/render/IRenderDataCollector.h"
#include "aurora/render/RenderEnvironment.h"

namespace aurora::render {
	ForwardRenderer::ForwardRenderer(modules::graphics::IGraphicsModule& graphics) :
		_graphics(graphics),
		_defaultBlendState(graphics.createBlendState()),
		_defaultDepthStencilState(graphics.createDepthStencilState()),
		_defaultRasterizerState(graphics.createRasterizerState()),
		_numLights(0),
		_nullLightData(new LightData()),
		_m34_l2w(new ShaderParameter()),
		_m34_l2v(new ShaderParameter()),
		_m44_l2p(new ShaderParameter()),
		_shaderDefines(new ShaderDefineCollection()),
		_shaderParameters(new ShaderParameterCollection()) {
		_shaderParameters->set(ShaderPredefine::MATRIX_LW, _m34_l2w);
		_shaderParameters->set(ShaderPredefine::MATRIX_LV, _m34_l2v);
		_shaderParameters->set(ShaderPredefine::MATRIX_LP, _m44_l2p);
	}

	void ForwardRenderer::collectRenderData(IRenderDataCollector& collector) {
		collector.data.renderer = this;

		auto renderable = collector.data.renderable;
		for (auto& pass : renderable->renderPasses) {
			if (pass && pass->material) {
				collector.data.priority = pass->priority;
				collector.data.state = pass->state;
				collector.data.material = pass->material;

				renderable->collectRenderData(collector);

				collector.data.priority.reset();
				collector.data.state = nullptr;
				collector.data.material = nullptr;
			}
		}

		collector.data.renderer = nullptr;
	}

	bool ForwardRenderer::collectRenderDataConfirm(IRenderDataCollector& collector) const {
		auto mesh = collector.data.mesh;
		return mesh && !mesh->getVertexBuffers().isEmpty() && mesh->getIndexBuffer();
	}

	void ForwardRenderer::preRender(const RenderEnvironment& env) {
		_numLights = 0;
		for (size_t i = 0, n = env.lights.size(); i < n; ++i) {
			auto& l = env.lights[i];
			if (l) {
				auto& data = _numLights >= _lightsData.size() ? _lightsData.emplace_back(new LightData()) : _lightsData[_numLights];
				
				if (l->isKindOf<components::lights::ILight>()) {
					++_numLights;

					data->color->set(l->getColor() * l->getIntensity());

					if (l->isKindOf<components::lights::DirectionLight>()) {
						auto& wm = l->getNode()->getWorldMatrix();
						Vec3f32 dir(wm[0][2], wm[1][2], wm[2][2]);
						dir.normalize();
						data->dir->set(dir);

						data->lightType = "1";
					} else if (l->isKindOf<components::lights::PointLight>()) {
						auto radius = ((const components::lights::PointLight*)l)->getRadius();

						data->pos->set(l->getNode()->getWorldPosition());
						data->attenuation->set(Vec3f32(1.f, 4.5f / radius, 75.f / (radius * radius)));

						data->lightType = "2";
					} else if (l->isKindOf<components::lights::SpotLight>()) {
						auto sl = (const components::lights::SpotLight*)l;
						auto radius = sl->getRadius();

						auto& wm = l->getNode()->getWorldMatrix();
						Vec3f32 dir(wm[0][2], wm[1][2], wm[2][2]);
						dir.normalize();
						data->dir->set(dir);

						data->pos->set(l->getNode()->getWorldPosition());
						data->attenuation->set(Vec4f32(1.f, 4.5f / radius, 75.f / (radius * radius), std::cos(sl->getSpotAngle() * 0.5f)));

						data->lightType = "3";
					} else {
						--_numLights;
					}
				}
			}
		}

		if (!_numLights) _shaderDefines->set("_LIGHT_TYPE", "0");
	}

	void ForwardRenderer::render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) {
		for (size_t i = 0; i < count; ++i) {
			auto rd = data[i];

			auto material = rd->material;
			if(!material) continue;

			auto shader = rd->material->getShader();
			if (!shader) continue;

			modules::graphics::IProgram* program;

			{
				if (_numLights) {
					auto& data = _lightsData[0];
					_shaderDefines->set("_LIGHT_TYPE", data->lightType);
					_shaderParameters->set("_light", data->param);
				}

				StackPopper<ShaderDefineGetterStack, StackPopperFlag::MULTI_POP> popper(shaderDefineStack, shaderDefineStack.push(&*_shaderDefines, rd->material->getDefines()));

				program = shader->select(&shaderDefineStack);
				if (!program) continue;
			}

			if (rd->state) {
				{
					auto& bs = rd->state->blend;
					_graphics->setBlendState(bs.state ? bs.state : _defaultBlendState, bs.constantFactors);
				}

				{
					auto& ds = rd->state->depthStencil;
					_graphics->setDepthStencilState(ds.state ? ds.state : _defaultDepthStencilState, ds.stencilFrontRef, ds.stencilBackRef);
				}

				{
					auto& rs = rd->state->rasterizer;
					_graphics->setRasterizerState(rs.state ? rs.state : _defaultRasterizerState);
				}
			} else {
				_graphics->setBlendState(_defaultBlendState, Vec4f32::ZERO);
				_graphics->setDepthStencilState(_defaultDepthStencilState, 0);
				_graphics->setRasterizerState(_defaultRasterizerState);
			}

			StackPopper<ShaderParameterGetterStack, StackPopperFlag::MULTI_POP> popper(shaderParameterStack, shaderParameterStack.push(&*_shaderParameters, rd->material->getParameters()));

			_m34_l2w->set(rd->matrix.l2w, ShaderParameterUpdateBehavior::FORCE);
			_m34_l2v->set(rd->matrix.l2v, ShaderParameterUpdateBehavior::FORCE);
			_m44_l2p->set(rd->matrix.l2p, ShaderParameterUpdateBehavior::FORCE);

			_graphics->draw(&rd->mesh->getVertexBuffers(), program, &shaderParameterStack, rd->mesh->getIndexBuffer());
		}
	}

	void ForwardRenderer::postRender() {
	}
}