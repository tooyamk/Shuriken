#pragma once

#include "modules/graphics/IGraphicsModule.h"
#include "modules/graphics/IProgramSourceTranslator.h"
#include "base/Application.h"
#include "utils/hash/xxHash.h"

#include "GL/glew.h"
#include <GL/GL.h>

//#include "GL/eglew.h"

namespace aurora::modules::graphics::win_gl {
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


	struct InternalRasterizerState {
		bool cullEnabled;
		uint8_t unuse = 0;
		uint16_t fillMode;
		uint16_t cullMode;
		uint16_t frontFace;
	};


	inline uint64_t calcHash(const void* data, size_t size) {
		return hash::xxHash::calc<64, std::endian::native>((uint8_t*)data, size, 0);
	}

	template<typename T>
	inline uint64_t calcHash(const T& val) {
		return calcHash(&val, sizeof(T));
	}
}