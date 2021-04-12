#pragma once

#include "HID.h"

#if AE_OS != AE_OS_WIN
namespace aurora::extensions {
	class HIDDeviceInfo {};
	class HIDDevice {};
}
#endif