#pragma once

#include "aurora/modules/inputs/IInputModule.h"
#include "aurora/IApplication.h"

#include <Xinput.h>
#include <shared_mutex>

namespace aurora::modules::inputs::xinput {
	struct InternalGUID {
		const char head[6] = { 'X', 'I', 'n', 'p', 'u', 't' };
		uint8_t index = 0;
	};
}