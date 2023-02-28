#pragma once

#include "srk/render/IRenderer.h"
#include "srk/modules/graphics/GraphicsModule.h"

namespace srk {
	class Material;
}

namespace srk::render {
	class SRK_FW_DLL SpriteRenderer : public IRenderer {
	public:
		SpriteRenderer(modules::graphics::IGraphicsModule& graphics);

		virtual void SRK_CALL collectRenderData(IRenderDataCollector& collector) override;
		virtual bool SRK_CALL collectRenderDataConfirm(IRenderDataCollector& collector) const override;
		virtual void SRK_CALL preRender(const RenderEnvironment& env) override;
		virtual void SRK_CALL render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) override;
		virtual void SRK_CALL postRender() override;

	protected:
		void SRK_CALL _flush(Material* matrial);
	};
}