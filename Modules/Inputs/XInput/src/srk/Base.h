#pragma once

#include "srk/modules/inputs/IInputModule.h"
#include "srk/applications/IApplication.h"

#include <Xinput.h>
#include <shared_mutex>

namespace srk::modules::inputs::xinput {
	struct InternalGUID {
		const char head[6] = { 'X', 'I', 'n', 'p', 'u', 't' };
		uint8_t index = 0;
	};
}