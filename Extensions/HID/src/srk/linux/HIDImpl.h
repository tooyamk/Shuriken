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

	class HIDUtils {
	public:
		static int32_t SRK_CALL parseUeventInfo(const char* uevent, uint32_t* busType, uint16_t* vid, uint16_t* pid, char** serialNumber, char** productName);

		static int32_t SRK_CALL getHidReportDescriptor(const char* rptPath, hidraw_report_descriptor* desc);
		static int32_t SRK_CALL getHidReportDescriptorFromSysfs(const char* sysfsPath, hidraw_report_descriptor* desc);

		static int32_t SRK_CALL getNextHidUsage(uint8_t* reportDesc, uint32_t size, uint32_t* pos, HIDUsagePage& usagePage, HIDUsage& usage);
		static int32_t SRK_CALL getHidItemSize(uint8_t* reportDesc, uint32_t pos, uint32_t size, int32_t* dataLen, int32_t* keySize);
		static uint32_t SRK_CALL getHidReportBytes(uint8_t* reportDesc, size_t len, size_t numBytes, size_t cur);
	};
}
#endif