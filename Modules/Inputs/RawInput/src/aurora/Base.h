#pragma once

#include "aurora/modules/inputs/IInputModule.h"
#include "aurora/IApplication.h"
#include <shared_mutex>

namespace aurora::modules::inputs::raw_input {
	struct InternalDeviceInfo : public DeviceInfo {
		HANDLE hDevice;
	};
}