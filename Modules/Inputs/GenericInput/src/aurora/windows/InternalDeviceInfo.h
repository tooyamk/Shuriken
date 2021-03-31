#pragma once

#if AE_OS == AE_OS_WIN
#include "Base.h"

namespace aurora::modules::inputs::generic_input {
	struct InternalDeviceInfo : public DeviceInfo {
		std::wstring devicePath;
	};
}
#endif