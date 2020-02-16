#pragma once

#include "aurora/Ref.h"

namespace aurora {
	class ShaderDefineGetterStack;
	class ShaderParameterGetterStack;
}

namespace aurora::render {
	class IRenderDataCollector;
	class RenderData;


	class AE_DLL IRenderer : public Ref {
	public:
		virtual ~IRenderer() {}

		//virtual bool AE_CALL checkValidity(const components::IRenderable& renderable) const = 0;
		virtual void AE_CALL collectRenderData(IRenderDataCollector& collector) const = 0;
		virtual bool AE_CALL collectRenderDataConfirm(IRenderDataCollector& collector) const = 0;
		virtual void AE_CALL render(RenderData*const* data, size_t count, ShaderDefineGetterStack& shaderDefineStack, ShaderParameterGetterStack& shaderParameterStack) const = 0;
	};
}