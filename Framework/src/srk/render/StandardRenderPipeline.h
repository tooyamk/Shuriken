#pragma once

#include "srk/components/IComponent.h"
#include "srk/render/IRenderPipeline.h"
#include "srk/Shader.h"
#include "srk/ShaderParameter.h"
#include "srk/math/Matrix.h"
#include "srk/render/IRenderCollector.h"
#include "srk/render/IRenderDataCollector.h"
#include "srk/render/RenderData.h"
#include "srk/render/RenderEnvironment.h"
#include <unordered_set>

namespace srk {
	class Material;
}

namespace srk::render {
	class IRenderer;


	class SRK_FW_DLL StandardRenderPipeline : public IRenderPipeline {
	public:
		StandardRenderPipeline();
		virtual ~StandardRenderPipeline();

		inline ShaderParameterCollection& getShaderParameters() const {
			return *_shaderParameters;
		}

		virtual void SRK_CALL render(modules::graphics::IGraphicsModule* graphics, const std::function<void(IRenderCollector&)>& fn) override;

	protected:
		class RenderCollector : public IRenderCollector {
		public:
			RenderCollector(StandardRenderPipeline& pipeline);

			virtual void SRK_CALL addCamera(components::Camera* camera) override;
			virtual void SRK_CALL addRenderable(components::renderables::IRenderable* renderable) override;
			virtual void SRK_CALL addLight(components::lights::ILight* light) override;

		private:
			StandardRenderPipeline& _pipeline;
		};


		class RenderDataCollector : public IRenderDataCollector {
		public:
			RenderDataCollector(StandardRenderPipeline& pipeline);

			virtual void SRK_CALL commit() override;

			struct {
				Matrix3x4f32 w2v;
				Matrix4x4f32 w2p;
			} matrix;

		private:
			StandardRenderPipeline& _pipeline;
		};


		void SRK_CALL _addCamera(components::Camera* camera);
		void SRK_CALL _addRenderable(components::renderables::IRenderable* renderable);
		void SRK_CALL _addLight(components::lights::ILight* light);

		inline bool SRK_CALL _isValidComponent(components::IComponent* com) const {
			return com && com->isEnalbed() && com->layer && com->getSceneNode();
		}

		void SRK_CALL _appendRenderData(RenderDataCollector& collector);
		void SRK_CALL _render();

		std::vector<components::Camera*> _cameras;
		std::vector<components::renderables::IRenderable*> _renderables;

		std::vector<RenderData*> _renderDataPool;
		size_t _renderDataPoolVernier;
		std::vector<RenderData*> _renderQueue;

		std::unordered_set<IRenderer*> _renderers;

		RenderEnvironment _renderEnv;

		struct {
			IntrusivePtr<ShaderParameter> m34_w2v;
			IntrusivePtr<ShaderParameter> m44_w2p;
			IntrusivePtr<ShaderParameter> v3_camPos;
		} _builtinShaderParameters;

		IntrusivePtr<ShaderParameterCollection> _shaderParameters;

		IntrusivePtr<ShaderDefineGetterStack> _shaderDefineStack;
		IntrusivePtr<ShaderParameterGetterStack> _shaderParameterStack;

		friend RenderDataCollector;
	};
}