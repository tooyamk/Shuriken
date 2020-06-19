#pragma once

#include "aurora/render/IRenderer.h"
#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora::render {
	class AE_DLL SpriteRenderer : public IRenderer {
	public:
		SpriteRenderer(modules::graphics::IGraphicsModule& graphics);

		virtual void AE_CALL collectRenderData(IRenderDataCollector& collector) override;
		virtual bool AE_CALL collectRenderDataConfirm(IRenderDataCollector& collector) const override;
		virtual void AE_CALL preRender(const RenderEnvironment& env) override;
		virtual void AE_CALL render(RenderData* const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) override;
		virtual void AE_CALL postRender() override;

	protected:
	};
}