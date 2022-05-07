#pragma once

#include "srk/modules/graphics/IGraphicsModule.h"

namespace srk::render {
	class SRK_FW_DLL RenderState {
		SRK_REF_OBJECT(RenderState)
	public:
		struct {
			IntrusivePtr<modules::graphics::IBlendState> state;
			Vec4f32 constantFactors;
		} blend;

		struct {
			IntrusivePtr<modules::graphics::IDepthStencilState> state;
			uint32_t stencilFrontRef = 0;
			uint32_t stencilBackRef = 0;
		} depthStencil;

		struct {
			IntrusivePtr<modules::graphics::IRasterizerState> state;
		} rasterizer;
	};
}