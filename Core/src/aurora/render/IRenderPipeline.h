#pragma once

#include "aurora/Node.h"

namespace aurora::render {
	class AE_DLL IRenderPipeline : public Ref {
	public:
		virtual ~IRenderPipeline() {}

		virtual void AE_CALL render(Node* node) = 0;
	};
}