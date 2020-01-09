#pragma once

#include "modules/graphics/IGraphicsModule.h"
#include "modules/graphics/IProgramSourceTranslator.h"
#include "base/Application.h"

#include "GL/glew.h"
#pragma comment (lib, "glew.lib")

#include <GL/GL.h>
#pragma comment (lib, "opengl32.lib")

#include "GL/eglew.h"

namespace aurora::modules::graphics::win_glew {
	static constexpr uint8_t MAX_RTS = 8;

	struct InternalBlendFunc {
		union {
			uint64_t featureValue;

			struct {
				uint16_t srcColor;
				uint16_t dstColor;
				uint16_t srcAlpha;
				uint16_t dstAlpha;
			};
		};
	};


	struct InternalBlendOp {
		union {
			uint32_t featureValue;

			struct {
				uint16_t color;
				uint16_t alpha;
			};
		};
	};


	struct InternalBlendWriteMask {
		union {
			uint32_t featureValue;
			bool rgba[4];
		};
	};

	
	struct InternalRenderTargetBlendState {
		RenderTargetBlendState state;
		InternalBlendFunc internalFunc;
		InternalBlendOp internalOp;
		InternalBlendWriteMask internalWriteMask;
	};
}