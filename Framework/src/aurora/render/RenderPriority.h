#pragma once

#include "aurora/Global.h"

namespace aurora::render {
	struct AE_FW_DLL RenderPriority {
		using Level1 = int32_t;

		enum class Level2 : int32_t {
			FOREMOST = -2,
			NEAR_TO_FAR = -1,
			MIDDLE = 0,
			FAR_TO_NEAR = 1,
			FINALLY = 2
		};


		RenderPriority() {
			reset();
		}

		Level1 level1;
		Level2 level2;

		inline void AE_CALL reset() {
			level1 = 0;
			level2 = Level2::MIDDLE;
		}
	};
}