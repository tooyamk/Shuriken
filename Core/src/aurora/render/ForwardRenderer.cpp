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
		_defaultBaseBlendState(graphics.createBlendState()),
		_defaultAddBlendState(graphics.createBlendState()),
		_defaultBaseDepthStencilState(graphics.createDepthStencilState()),
		_defaultAddDepthStencilState(graphics.createDepthStencilState()),
		_defaultRasterizerState(graphics.createRasterizerState()),
		_numLights(0),
		_baseTag("forward_base"),
		_addTag("forward_add"),
		_m34_l2w(new ShaderParameter()),
		_m34_l2v(new ShaderParameter()),
		_m44_l2p(new ShaderParameter()),
		_shaderDefines(new ShaderDefineCollection()),
		_shaderParameters(new ShaderParameterCollection()) {
		{
			modules::graphics::RenderTargetBlendState bs;
			bs.enabled = true;
			bs.func = modules::graphics::BlendFunc(modules::graphics::BlendFactor::ONE, modules::graphics::BlendFactor::ONE);
			_defaultAddBlendState->setRenderTargetState(0, bs);
		}
		{
			modules::graphics::DepthState ds;
			ds.func = modules::graphics::ComparisonFunc::LESS_EQUAL;
			ds.writeable = false;
			_defaultAddDepthStencilState->setDepthState(ds);
		}

		_shaderParameters->set(ShaderPredefine::MATRIX_LW, _m34_l2w);
		_shaderParameters->set(ShaderPredefine::MATRIX_LV, _m34_l2v);
		_shaderParameters->set(ShaderPredefine::MATRIX_LP, _m44_l2p);

		_setLightType(LIGHT_TYPE_NONE);
		_renderFn = &ForwardRenderer::_render<0>;
	}

	void ForwardRenderer::collectRenderData(IRenderDataCollector& collector) {
		auto& data = collector.data;
		data.renderer = this;

		auto renderable = data.renderable;
		for (auto& pass : renderable->renderPasses) {
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
		}

		data.renderer = nullptr;
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
				
				++_numLights;

				data->color->set(l->getColor() * l->getIntensity());

				if (l->isKindOf<components::lights::DirectionLight>()) {
					auto& wm = l->getNode()->getWorldMatrix();
					Vec3f32 dir(wm[0][2], wm[1][2], wm[2][2]);
					dir.normalize();
					data->dir->set(dir);

					data->lightType = LIGHT_TYPE_DIRECTION;
				} else if (l->isKindOf<components::lights::PointLight>()) {
					auto radius = ((const components::lights::PointLight*)l)->getRadius();

					data->pos->set(l->getNode()->getWorldPosition());
					data->attenuation->set(Vec3f32(1.f, 4.5f / radius, 75.f / (radius * radius)));

					data->lightType = LIGHT_TYPE_POINT;
				} else if (l->isKindOf<components::lights::SpotLight>()) {
					auto sl = (const components::lights::SpotLight*)l;
					auto radius = sl->getRadius();

					auto& wm = l->getNode()->getWorldMatrix();
					Vec3f32 dir(wm[0][2], wm[1][2], wm[2][2]);
					dir.normalize();
					data->dir->set(dir);

					data->pos->set(l->getNode()->getWorldPosition());
					data->attenuation->set(Vec4f32(1.f, 4.5f / radius, 75.f / (radius * radius), std::cos(sl->getSpotAngle() * 0.5f)));

					data->lightType = LIGHT_TYPE_SPOT;
				} else {
					--_numLights;
				}
			}
		}

		if (_numLights) {
			if (_numLights > 1) {
				_renderFn = &ForwardRenderer::_render<2>;
			} else {
				_renderFn = &ForwardRenderer::_render<1>;
			}
		} else {
			_setLightType(LIGHT_TYPE_NONE);
			_renderFn = &ForwardRenderer::_render<0>;
		}
	}

	void ForwardRenderer::render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) {
		for (size_t i = 0; i < count; ++i) {
			auto rd = data[i];

			{
				auto& m = rd->matrix;
				_m34_l2w->set(m.l2w, ShaderParameterUpdateBehavior::FORCE);
				_m34_l2v->set(m.l2v, ShaderParameterUpdateBehavior::FORCE);
				_m44_l2p->set(m.l2p, ShaderParameterUpdateBehavior::FORCE);
			}

			(this->*_renderFn)(rd, shaderDefineStack, shaderParameterStack);
		}
	}

	void ForwardRenderer::_render(Material* material, RenderState* state, const Mesh* mesh, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack,
		modules::graphics::IBlendState* defaultBlendState, modules::graphics::IDepthStencilState* defaultDepthStencilState) {
		if (!material) return;

		auto shader = material->getShader();
		if (!shader) return;

		modules::graphics::IProgram* program;

		{
			StackPopper<ShaderDefineGetterStack, StackPopperFlag::MULTI_POP> popper(shaderDefineStack, shaderDefineStack.push(&*_shaderDefines, material->getDefines()));

			program = shader->select(&shaderDefineStack);
			if (!program) return;
		}

		if (state) {
			{
				auto& bs = state->blend;
				_graphics->setBlendState(bs.state ? bs.state : defaultBlendState, bs.constantFactors);
			}

			{
				auto& ds = state->depthStencil;
				_graphics->setDepthStencilState(ds.state ? ds.state : defaultDepthStencilState, ds.stencilFrontRef, ds.stencilBackRef);
			}

			{
				auto& rs = state->rasterizer;
				_graphics->setRasterizerState(rs.state ? rs.state : _defaultRasterizerState);
			}
		} else {
			_graphics->setBlendState(defaultBlendState, Vec4f32::ZERO);
			_graphics->setDepthStencilState(defaultDepthStencilState, 0);
			_graphics->setRasterizerState(_defaultRasterizerState);
		}

		StackPopper<ShaderParameterGetterStack, StackPopperFlag::MULTI_POP> popper(shaderParameterStack, shaderParameterStack.push(&*_shaderParameters, material->getParameters()));

		_graphics->draw(&mesh->getVertexBuffers(), program, &shaderParameterStack, mesh->getIndexBuffer());
	}

	void ForwardRenderer::postRender() {
	}
}