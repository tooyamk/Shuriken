#pragma once

#include "aurora/render/IRenderer.h"
#include "aurora/Mesh.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include "aurora/render/RenderData.h"
#include "aurora/render/RenderTag.h"

namespace aurora {
	class Material;
}

namespace aurora::render {
	class RenderState;


	class AE_FW_DLL ForwardRenderer : public IRenderer {
	public:
		ForwardRenderer(modules::graphics::IGraphicsModule& graphics);

		//virtual bool AE_CALL checkValidity(const components::IRenderable& renderable) const override;
		virtual void AE_CALL collectRenderData(IRenderDataCollector& collector) override;
		virtual bool AE_CALL collectRenderDataConfirm(IRenderDataCollector& collector) const override;
		virtual void AE_CALL preRender(const RenderEnvironment& env) override;
		virtual void AE_CALL render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) override;
		virtual void AE_CALL postRender() override;

	protected:
		struct LightData {
			AE_REF_OBJECT(LightData)
		public:
			LightData() :
				param(new ShaderParameter()),
				paramCollection(new ShaderParameterCollection()),
				color(new ShaderParameter()),
				dir(new ShaderParameter()),
				pos(new ShaderParameter()),
				attenuation(new ShaderParameter()) {
				param->set(paramCollection);
				paramCollection->set("color", color);
				paramCollection->set("dir", dir);
				paramCollection->set("pos", pos);
				paramCollection->set("attenuation", attenuation);
			}

			std::string_view lightType;

			IntrusivePtr<ShaderParameter> param;
			IntrusivePtr<ShaderParameterCollection> paramCollection;

			IntrusivePtr<ShaderParameter> color;
			IntrusivePtr<ShaderParameter> dir;
			IntrusivePtr<ShaderParameter> pos;
			IntrusivePtr<ShaderParameter> attenuation;

			void AE_CALL reset() {
				//todo
			}
		};


		inline static const std::string LIGHT_TYPE = { "_LIGHT_TYPE" };
		inline static const std::string LIGHT_TYPE_NONE = { "0" };
		inline static const std::string LIGHT_TYPE_DIRECTION = { "1" };
		inline static const std::string LIGHT_TYPE_POINT = { "2" };
		inline static const std::string LIGHT_TYPE_SPOT = { "3" };
		inline static const std::string LIGHT = { "_light" };

		IntrusivePtr<modules::graphics::IGraphicsModule> _graphics;

		IntrusivePtr<modules::graphics::IBlendState> _defaultBaseBlendState;
		IntrusivePtr<modules::graphics::IBlendState> _defaultAddBlendState;
		IntrusivePtr<modules::graphics::IDepthStencilState> _defaultBaseDepthStencilState;
		IntrusivePtr<modules::graphics::IDepthStencilState> _defaultAddDepthStencilState;
		IntrusivePtr<modules::graphics::IRasterizerState> _defaultRasterizerState;

		RenderTag _baseTag;
		RenderTag _addTag;

		std::vector<IntrusivePtr<LightData>> _lightsData;
		size_t _numLights;
		std::string_view _curLightType;

		IntrusivePtr<ShaderParameter> _m34_l2w;
		IntrusivePtr<ShaderParameter> _m34_l2v;
		IntrusivePtr<ShaderParameter> _m44_l2p;

		IntrusivePtr<ShaderDefineCollection> _shaderDefines;
		IntrusivePtr<ShaderParameterCollection> _shaderParameters;

		void(AE_CALL ForwardRenderer::*_renderFn)(RenderData*, ShaderDefineGetterStack&, ShaderParameterGetterStack&);

		inline void AE_CALL _switchLight(size_t index) {
			auto& data = _lightsData[index];
			_setLightType(data->lightType);
			_shaderParameters->set(LIGHT, data->param);
		}

		inline void AE_CALL _setLightType(const std::string_view& type) {
			if (_curLightType != type) {
				_curLightType = type;
				_shaderDefines->set(LIGHT_TYPE, type);
			}
		}

		void AE_CALL _render(Material* material, RenderState* state, const MeshBuffer* meshBuffer, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack, 
			modules::graphics::IBlendState* defaultBlendState, modules::graphics::IDepthStencilState* defaultDepthStencilState);

		template<size_t N>
		void AE_CALL _render(RenderData* rd, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) {
			if (auto mesh = rd->mesh(); mesh) {
				if (auto meshBuffer = mesh->getBuffer(); meshBuffer) {
					if constexpr (N > 0) {
						_switchLight(0);
						_render(rd->material, rd->state, meshBuffer, shaderDefineStack, shaderParameterStack, _defaultBaseBlendState, _defaultBaseDepthStencilState);

						if constexpr (N > 1) {
							if (rd->subPasses && !rd->subPasses->empty()) {
								for (auto& p : *rd->subPasses) {
									if (p && p->tags && p->tags->has(_addTag)) {
										for (size_t i = 1; i < _numLights; ++i) {
											_switchLight(i);
											_render(p->material, p->state, meshBuffer, shaderDefineStack, shaderParameterStack, _defaultAddBlendState, _defaultAddDepthStencilState);
										}
									}
								}
							}
						}
					} else {
						_render(rd->material, rd->state, meshBuffer, shaderDefineStack, shaderParameterStack, _defaultBaseBlendState, _defaultBaseDepthStencilState);
					}
				}
			}
		}
	};
}