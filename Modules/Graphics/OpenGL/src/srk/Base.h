#pragma once

#include "srk/modules/graphics/GraphicsModule.h"

#include "GL/glew.h"

#if SRK_OS == SRK_OS_WINDOWS
#	include <GL/GL.h>
#	include "GL/wglew.h"
#elif SRK_OS == SRK_OS_LINUX
#	include <GL/glx.h>
#endif

#include <optional>

namespace srk::modules::graphics::gl {
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


	struct InternalRasterizerState {
		bool cullEnabled;
		bool scissorEnabled;
		uint16_t fillMode;
		uint16_t cullMode;
		uint16_t frontFace;
	};


	struct InternalDepthState {
		InternalDepthState() :
			featureValue(0) {
		}

		union {
			uint32_t featureValue;

			struct {
				bool enabled;
				bool writeable;
				uint16_t func;
			};
		};
	};


	struct InternalStencilFaceState {
		InternalStencilFaceState() :
			ref(0),
			func(GL_NEVER) {
			mask.featureValue = 0;
			op.featureValue = 0;
		}

		uint8_t ref;
		uint16_t func;

		struct {
			union {
				uint16_t featureValue;

				uint8_t read;
				uint8_t write;
			};
		} mask;

		struct {
			union {
				uint64_t featureValue;

				struct {
					uint16_t fail;
					uint16_t depthFail;
					uint16_t pass;
					uint16_t reserved;
				};
			};
		} op;
	};


	struct InternalStencilState {
		bool enabled;
		
		struct {
			InternalStencilFaceState front;
			InternalStencilFaceState back;
		} face;
	};

	inline bool SRK_CALL glInit() {
		return glewInit() == GLEW_OK;
	}

	inline bool SRK_CALL glIsSupported(const char* name) {
		return glewIsSupported(name);
	}
}