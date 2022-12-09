#pragma once

#include "srk/modules/graphics/IGraphicsModule.h"

namespace srk::render {
	class SRK_FW_DLL RenderState {
		SRK_REF_OBJECT(RenderState)
	public:
		struct {
			IntrusivePtr<modules::graphics::IBlendState> state;
		} blend;

		struct {
			IntrusivePtr<modules::graphics::IDepthStencilState> state;
		} depthStencil;

		struct {
			IntrusivePtr<modules::graphics::IRasterizerState> state;
		} rasterizer;
	};
}