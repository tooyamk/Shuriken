#pragma once

#include "srk/modules/inputs/InputModule.h"
#include "srk/HID.h"
#include <shared_mutex>

namespace srk::modules::inputs::hid_input {
	class SRK_MODULE_DLL InternalDeviceInfo : public DeviceInfo {
	public:
		std::string path;
		int32_t index;
	};
}