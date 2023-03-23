#pragma once

#include "srk/modules/inputs/InputModule.h"
#include "srk/modules/windows/WindowModule.h"
#include <shared_mutex>

namespace srk::modules::inputs::evdev {
	struct InternalDeviceInfo : public DeviceInfo {
		char eventNumber[4];
	};
}