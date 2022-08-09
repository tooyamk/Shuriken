#pragma once

#include "srk/modules/inputs/IInputModule.h"
#include "srk/modules/windows/IWindowModule.h"
#include <shared_mutex>

namespace srk::modules::inputs::raw_input {
	struct InternalDeviceInfo : public DeviceInfo {
		HANDLE hDevice;
	};
}