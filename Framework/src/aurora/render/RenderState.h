#pragma once

#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora::render {
	class AE_FW_DLL RenderState {
		AE_REF_OBJECT(RenderState)
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