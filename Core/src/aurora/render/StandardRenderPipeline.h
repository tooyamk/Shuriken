#pragma once

#include "aurora/render/IRenderPipeline.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"
#include "aurora/math/Matrix.h"
#include "aurora/render/IRenderCollector.h"
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

		virtual void AE_CALL render(modules::graphics::IGraphicsModule* graphics, const std::function<void(IRenderCollector&)>& fn) override;

	protected:
		class RenderCollector : public IRenderCollector {
		public:
			RenderCollector(StandardRenderPipeline& pipeline);

			virtual void AE_CALL addCamera(components::Camera* camera) override;
			virtual void AE_CALL addRenderable(components::renderables::IRenderable* renderable) override;
			virtual void AE_CALL addLight(components::lights::ILight* light) override;

		private:
			StandardRenderPipeline& _pipeline;
		};


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


		void AE_CALL _addCamera(components::Camera* camera);
		void AE_CALL _addRenderable(components::renderables::IRenderable* renderable);
		void AE_CALL _addLight(components::lights::ILight* light);

		void AE_CALL _appendRenderData(RenderDataCollector& collector);
		void AE_CALL _render();

		std::vector<components::Camera*> _cameras;
		std::vector<components::renderables::IRenderable*> _renderables;

		std::vector<RenderData*> _renderDataPool;
		size_t _renderDataPoolVernier;
		std::vector<RenderData*> _renderQueue;

		std::unordered_set<IRenderer*> _renderers;

		RenderEnvironment _renderEnv;

		struct {
			RefPtr<ShaderParameter> m34_w2v;
			RefPtr<ShaderParameter> m44_w2p;
			RefPtr<ShaderParameter> v3_camPos;
		} _builtinShaderParameters;

		RefPtr<ShaderParameterCollection> _shaderParameters;

		RefPtr<ShaderDefineGetterStack> _shaderDefineStack;
		RefPtr<ShaderParameterGetterStack> _shaderParameterStack;

		friend RenderDataCollector;
	};
}