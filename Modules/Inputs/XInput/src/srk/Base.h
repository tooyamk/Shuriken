#pragma once

#include "srk/modules/inputs/InputModule.h"

#include <Xinput.h>
#include <shared_mutex>

namespace srk::modules::inputs::xinput {
	struct InternalGUID {
		const char head[6] = { 'X', 'I', 'n', 'p', 'u', 't' };
		uint16_t vendorID = 0;
		uint16_t productID = 0;
		uint8_t index = 0;
		uint8_t unused = 0;
	};
}