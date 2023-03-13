#pragma once

#include "HID.h"

#if SRK_OS == SRK_OS_LINUX
struct hidraw_report_descriptor;

namespace srk::extensions {
	class HIDDeviceInfo {
	public:
		HIDDeviceInfo();

		bool valid;
		uint16_t vendorID;
		uint16_t productID;
		HIDUsagePage usagePage;
		HIDUsage usage;
		std::string_view pathView;
		std::string manufacturer;
		std::string product;
	};


	class HIDDevice {
	public:
		HIDDevice(int32_t handle);

		int32_t handle;
	};
}
#endif