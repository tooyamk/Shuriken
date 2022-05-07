#pragma once

#include "HID.h"

#if SRK_OS != SRK_OS_WINDOWS && SRK_OS != SRK_OS_LINUX
namespace srk::extensions {
	class HIDDeviceInfo {};
	class HIDDevice {};
}
#endif