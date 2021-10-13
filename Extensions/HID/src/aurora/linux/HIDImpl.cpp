#include "linux/HIDImpl.h"

#if AE_OS == AE_OS_LINUX
#include "aurora/Debug.h"

#include <libudev.h>

namespace aurora::extensions {
	void HID::enumDevices(void*, HID::EnumDevicesCallback callback) {
		if (!callback) return;

		auto udev = udev_new();
		auto enumerate = udev_enumerate_new(udev);

		udev_enumerate_add_match_subsystem(enumerate, "hidraw");
		udev_enumerate_scan_devices(enumerate);

		for (auto device = udev_enumerate_get_list_entry(enumerate); device; device = udev_list_entry_get_next(device)) {
			uint32_t busType;
			uint16_t vid, pid;
			char *serialNumber = nullptr, *productName = nullptr;

			auto sysfsPath = udev_list_entry_get_name(device);
			auto rawDev = udev_device_new_from_syspath(udev, sysfsPath);
			auto hidDev = udev_device_get_parent_with_subsystem_devtype(rawDev, "hid", nullptr);
			if (!hidDev) goto next;

			//if (!parse_uevent_info(udev_device_get_sysattr_value(hidDev, "uevent"), &busType, &vid, &pid, &serialNumber, &productName)) goto next;

		next:
			udev_device_unref(rawDev);
		}

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		printaln(123);
	}

	bool HID::isValid(const HIDDeviceInfo& info) {
		return false;
	}

	bool HID::isValid(const HIDDevice& device) {
		return false;
	}

	uint16_t HID::getVendorID(const HIDDeviceInfo& info) {
		return 0;
	}

	uint16_t HID::getProductID(const HIDDeviceInfo& info) {
		return 0;
	}

	std::wstring HID::getManufacturerString(const HIDDeviceInfo& info) {
		return std::wstring();
	}

	std::wstring HID::getProductString(const HIDDeviceInfo& info) {
		return std::wstring();
	}

	std::string_view HID::getPath(const HIDDeviceInfo& info) {
		return std::string_view();
	}

	uint16_t HID::getUsagePage(const HIDDeviceInfo& info) {
		return 0;
	}

	uint16_t HID::getUsage(const HIDDeviceInfo& info) {
		return 0;
	}

	HIDDevice* HID::open(const std::string_view& path) {
		return nullptr;
	}

	void HID::close(HIDDevice& device) {}

	size_t HID::read(HIDDevice& device, void* data, size_t dataLength, size_t timeout) {
		return HID::OUT_ERROR;
	}

	size_t HID::write(HIDDevice& device, const void* data, size_t dataLength, size_t timeout) {
		return HID::OUT_ERROR;
	}

	void* HID::getPreparsedData(const HIDDeviceInfo& device) {
		return nullptr;
	}
	void* HID::getPreparsedData(const HIDDevice& device) {
		return nullptr;
	}
}
#endif