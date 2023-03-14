#include "empty/HIDImpl.h"

#if SRK_OS != SRK_OS_WINDOWS && SRK_OS != SRK_OS_LINUX
namespace srk::extensions {
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

	std::wstring_view HID::getManufacturerString(const HIDDeviceInfo& info) {
		return std::wstring_view();
	}

	std::wstring_view HID::getProductString(const HIDDeviceInfo& info) {
		return std::wstring_view();
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

	int32_t HID::getIndex(const HIDDeviceInfo& info) {
		return -1;
	}

	HIDDevice* HID::open(const std::string_view& path) {
		return nullptr;
	}

	void HID::close(HIDDevice& device) {
	}

	ByteArray HID::getReportDescriptor(const HIDDevice& device) {
		return ByteArray();
	}

	size_t HID::read(HIDDevice& device, void* data, size_t dataLength, size_t timeout) {
		return HID::OUT_ERROR;
	}

	size_t HID::write(HIDDevice& device, const void* data, size_t dataLength, size_t timeout) {
		return HID::OUT_ERROR;
	}
}
#endif