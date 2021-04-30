#include "empty/HIDImpl.h"

#if AE_OS != AE_OS_WIN
namespace aurora::extensions {
	void HID::enumDevices(void*, EnumDevicesCallback) {}

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