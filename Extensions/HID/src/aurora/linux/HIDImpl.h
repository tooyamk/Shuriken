#pragma once

#include "HID.h"

#if AE_OS == AE_OS_LINUX
namespace aurora::extensions {
	class HIDDeviceInfo {};
	class HIDDevice {};
}
#endif