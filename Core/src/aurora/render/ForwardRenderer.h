#pragma once

#include "aurora/render/IRenderer.h"
#include "aurora/ShaderParameter.h"
#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora::render {
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
		RefPtr<modules::graphics::IGraphicsModule> _graphics;

		RefPtr<modules::graphics::IBlendState> _defaultBlendState;
		RefPtr<modules::graphics::IDepthStencilState> _defaultDepthStencilState;
		RefPtr<modules::graphics::IRasterizerState> _defaultRasterizerState;

		RefPtr<ShaderParameter> _m34_l2w;
		RefPtr<ShaderParameter> _m34_l2v;
		RefPtr<ShaderParameter> _m44_l2p;

		RefPtr<ShaderParameterCollection> _shaderParameters;
	};
}