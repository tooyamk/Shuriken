#pragma once

#include "aurora/Node.h"

namespace aurora::renderers {
	class AE_DLL IRenderPipeline : public Ref {
	public:
		virtual ~IRenderPipeline() {}

		virtual void AE_CALL render(Node* node) = 0;
	};
}