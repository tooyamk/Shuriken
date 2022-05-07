#pragma once

#include "srk/Intrusive.h"

namespace srk {
	class ShaderDefineGetterStack;
	class ShaderParameterGetterStack;
}

namespace srk::render {
	class IRenderDataCollector;
	class RenderData;
	class RenderEnvironment;


	class SRK_FW_DLL IRenderer : public Ref {
	public:
		virtual ~IRenderer() {}

		//virtual bool SRK_CALL checkValidity(const components::IRenderable& renderable) const = 0;
		virtual void SRK_CALL collectRenderData(IRenderDataCollector& collector) = 0;
		virtual bool SRK_CALL collectRenderDataConfirm(IRenderDataCollector& collector) const = 0;
		virtual void SRK_CALL preRender(const RenderEnvironment& env) = 0;
		virtual void SRK_CALL render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) = 0;
		virtual void SRK_CALL postRender() = 0;
	};
}