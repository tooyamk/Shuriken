#pragma once

#include "aurora/render/IRenderer.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include "aurora/render/RenderTag.h"

namespace aurora {
	class Material;
	class Mesh;
}

namespace aurora::render {
	class RenderState;


	class AE_DLL ForwardRenderer : public IRenderer {
	public:
		ForwardRenderer(modules::graphics::IGraphicsModule& graphics);

		//virtual bool AE_CALL checkValidity(const components::IRenderable& renderable) const override;
		virtual void AE_CALL collectRenderData(IRenderDataCollector& collector) override;
		virtual bool AE_CALL collectRenderDataConfirm(IRenderDataCollector& collector) const override;
		virtual void AE_CALL preRender(const RenderEnvironment& env) override;
		virtual void AE_CALL render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) override;
		virtual void AE_CALL postRender() override;

	protected:
		struct LightData : public Ref {
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

			std::string lightType;

			RefPtr<ShaderParameter> param;
			RefPtr<ShaderParameterCollection> paramCollection;

			RefPtr<ShaderParameter> color;
			RefPtr<ShaderParameter> dir;
			RefPtr<ShaderParameter> pos;
			RefPtr<ShaderParameter> attenuation;

			void AE_CALL reset() {
				//todo
			}
		};


		RefPtr<modules::graphics::IGraphicsModule> _graphics;

		RefPtr<modules::graphics::IBlendState> _defaultBaseBlendState;
		RefPtr<modules::graphics::IBlendState> _defaultAddBlendState;
		RefPtr<modules::graphics::IDepthStencilState> _defaultBaseDepthStencilState;
		RefPtr<modules::graphics::IDepthStencilState> _defaultAddDepthStencilState;
		RefPtr<modules::graphics::IRasterizerState> _defaultRasterizerState;

		RenderTag _baseTag;
		RenderTag _addTag;

		std::vector<RefPtr<LightData>> _lightsData;
		size_t _numLights;
		RefPtr<LightData> _nullLightData;

		RefPtr<ShaderParameter> _m34_l2w;
		RefPtr<ShaderParameter> _m34_l2v;
		RefPtr<ShaderParameter> _m44_l2p;

		RefPtr<ShaderDefineCollection> _shaderDefines;
		RefPtr<ShaderParameterCollection> _shaderParameters;

		void AE_CALL _switchLight(size_t index);

		void AE_CALL _render(Material* material, RenderState* state, const Mesh* mesh, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack, 
			modules::graphics::IBlendState* defaultBlendState, modules::graphics::IDepthStencilState* defaultDepthStencilState);
	};
}