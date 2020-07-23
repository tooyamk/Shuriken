#pragma once

#include "aurora/render/RenderData.h"

namespace aurora::render {
	class AE_FW_DLL IRenderDataCollector {
	public:
		IRenderDataCollector() {
			reset();
		}
		virtual ~IRenderDataCollector() {}

		RenderData data;

		virtual void AE_CALL commit() = 0;

		inline void AE_CALL reset() {
			data.reset();
		}
	};
}