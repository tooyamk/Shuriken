#pragma once

#include "srk/modules/inputs/InputModule.h"
#include "srk/HID.h"

namespace srk::modules::inputs::hid_input {
	class SRK_MODULE_DLL InternalDeviceInfo : public DeviceInfo {
	public:
		std::string path;
		int32_t index;
	};
}