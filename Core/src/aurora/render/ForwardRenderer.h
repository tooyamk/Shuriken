#pragma once

#include "aurora/render/IRenderer.h"
#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora::render {
	class AE_DLL ForwardRenderer : public IRenderer {
	public:
		ForwardRenderer(modules::graphics::IGraphicsModule& graphics);

		//virtual bool AE_CALL checkValidity(const components::IRenderable& renderable) const override;
		virtual void AE_CALL collectRenderData(IRenderDataCollector& collector) const override;
		virtual bool AE_CALL collectRenderDataConfirm(IRenderDataCollector& collector) const override;
		virtual void AE_CALL render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) const override;

	protected:
		RefPtr<modules::graphics::IGraphicsModule> _graphics;

		RefPtr<modules::graphics::IBlendState> _defaultBlendState;
		RefPtr<modules::graphics::IDepthStencilState> _defaultDepthStencilState;
		RefPtr<modules::graphics::IRasterizerState> _defaultRasterizerState;
	};
}