#pragma once

#include "aurora/modules/graphics/IGraphicsModule.h"

namespace aurora::render {
	class AE_DLL RenderState : public Ref {
	public:
		struct {
			RefPtr<modules::graphics::IBlendState> state;
			Vec4f32 constantFactors;
		} blend;

		struct {
			RefPtr<modules::graphics::IDepthStencilState> state;
			uint32_t stencilFrontRef = 0;
			uint32_t stencilBackRef = 0;
		} depthStencil;

		struct {
			RefPtr<modules::graphics::IRasterizerState> state;
		} rasterizer;
	};
}