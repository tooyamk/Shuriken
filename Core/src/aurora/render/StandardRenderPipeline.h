#pragma once

#include "aurora/render/IRenderPipeline.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"
#include "aurora/render/IRenderDataCollector.h"
#include "aurora/render/RenderData.h"

namespace aurora {
	class Material;
}

namespace aurora::components {
	class IRenderable;
}

namespace aurora::render {
	class IRenderer;


	class AE_DLL StandardRenderPipeline : public IRenderPipeline {
	public:
		StandardRenderPipeline();

		virtual void AE_CALL render(Node* node) override;

	protected:
		class RenderDataCollector : public IRenderDataCollector {
		public:
			RenderDataCollector(StandardRenderPipeline& pipeline);

			virtual void AE_CALL commit() override;

		private:
			StandardRenderPipeline& _pipeline;
		};


		void AE_CALL _collectNode(Node* node, RenderDataCollector& collector);
		void AE_CALL _appendRenderData(RenderDataCollector& collector);
		void AE_CALL _render();

		std::vector<RenderData*> _renderDataPool;
		size_t _renderDataPoolVernier;
		std::vector<RenderData*> _renderQueue;

		ShaderDefineGetterStack _shaderDefineStack;
		ShaderParameterGetterStack _shaderParameterStack;

		friend RenderDataCollector;
	};
}