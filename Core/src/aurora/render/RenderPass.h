#pragma once

#include "aurora/Material.h"
#include "aurora/render/RenderPriority.h"
#include "aurora/render/RenderState.h"

namespace aurora::render {
	class AE_DLL RenderPass : public Ref {
	public:
		RenderPriority priority;
		RefPtr<RenderState> state;
		RefPtr<Material> material;
	};
}