#pragma once

#include "aurora/render/IRenderPipeline.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"
#include "aurora/math/Matrix.h"
#include "aurora/render/IRenderDataCollector.h"
#include "aurora/render/RenderData.h"
#include "aurora/render/RenderEnvironment.h"
#include <unordered_set>

namespace aurora {
	class Material;
}

namespace aurora::render {
	class IRenderer;


	class AE_DLL StandardRenderPipeline : public IRenderPipeline {
	public:
		StandardRenderPipeline();

		inline ShaderParameterCollection& getShaderParameters() const {
			return *_shaderParameters;
		}

		virtual void AE_CALL render(modules::graphics::IGraphicsModule* graphics, components::Camera* camera, Node* node, const std::vector<components::lights::ILight*>* lights) override;

	protected:
		class RenderDataCollector : public IRenderDataCollector {
		public:
			RenderDataCollector(StandardRenderPipeline& pipeline);

			virtual void AE_CALL commit() override;

			struct {
				Matrix34 w2v;
				Matrix44 w2p;
			} matrix;

		private:
			StandardRenderPipeline& _pipeline;
		};


		void AE_CALL _collectNode(Node* node, RenderDataCollector& collector);
		void AE_CALL _appendRenderData(RenderDataCollector& collector);
		void AE_CALL _render();

		std::vector<RenderData*> _renderDataPool;
		size_t _renderDataPoolVernier;
		std::vector<RenderData*> _renderQueue;

		std::unordered_set<IRenderer*> _renderers;

		RenderEnvironment _renderEnv;

		RefPtr<ShaderParameter> _m34_w2v;
		RefPtr<ShaderParameter> _m44_w2p;

		RefPtr<ShaderParameterCollection> _shaderParameters;

		RefPtr<ShaderDefineGetterStack> _shaderDefineStack;
		RefPtr<ShaderParameterGetterStack> _shaderParameterStack;

		friend RenderDataCollector;
	};
}