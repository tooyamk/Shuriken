#pragma once

#include "srk/render/RenderData.h"

namespace srk::render {
	class SRK_FW_DLL IRenderDataCollector {
	public:
		IRenderDataCollector() {
			reset();
		}
		virtual ~IRenderDataCollector() {}

		RenderData data;

		virtual void SRK_CALL commit() = 0;

		inline void SRK_CALL reset() {
			data.reset();
		}
	};
}