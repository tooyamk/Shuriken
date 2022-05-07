#pragma once

#include "srk/Material.h"
#include "srk/render/RenderPriority.h"
#include "srk/render/RenderState.h"
#include "srk/render/RenderTag.h"
//#include <map>
#include <set>

namespace srk::render {
	class SRK_FW_DLL RenderPass {
		SRK_REF_OBJECT(RenderPass)
	public:
		RenderPriority priority;
		IntrusivePtr<RenderState> state;
		IntrusivePtr<Material> material;

		IntrusivePtr<RenderTagCollection> tags;
		std::vector<IntrusivePtr<RenderPass>> subPasses;
		//std::map<RenderTag, RefPtr<RenderPass>, RenderTag::std_compare> subPasses;
	};
}