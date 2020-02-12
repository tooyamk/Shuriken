#pragma once

#include "aurora/renderers/IRenderPipeline.h"

namespace aurora::components {
	class IRenderable;
}

namespace aurora::renderers {
	class AE_DLL StandardRenderPipeline : public IRenderPipeline {
	public:
		virtual void AE_CALL render(Node* node) override;

	protected:
		void AE_CALL _collectNode(Node* node);

		std::vector<components::IRenderable*> _renderables;
	};
}