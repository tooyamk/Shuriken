#pragma once

#include "aurora/Material.h"
#include "aurora/render/RenderPriority.h"
#include "aurora/render/RenderState.h"
#include "aurora/render/RenderTag.h"
//#include <map>
#include <set>

namespace aurora::render {
	class AE_FW_DLL RenderPass : public Ref {
	public:
		RenderPriority priority;
		IntrusivePtr<RenderState> state;
		IntrusivePtr<Material> material;

		IntrusivePtr<RenderTagCollection> tags;
		std::vector<IntrusivePtr<RenderPass>> subPasses;
		//std::map<RenderTag, RefPtr<RenderPass>, RenderTag::std_compare> subPasses;
	};
}