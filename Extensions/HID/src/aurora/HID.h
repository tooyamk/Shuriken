#pragma once

#include "aurora/Global.h"

namespace aurora::extensions {
	class HIDDeviceInfo;
	class HIDDevice;

	class AE_EXTENSION_DLL HID {
	public:
		static constexpr size_t READ_IN_TIMEOUT_BLOCKING = (std::numeric_limits<size_t>::max)();
		static constexpr size_t READ_OUT_ERROR = (std::numeric_limits<size_t>::max)();
		static constexpr size_t READ_OUT_WAITTING = READ_OUT_ERROR - 1;

		using EnumDevicesCallback = void(*)(const HIDDeviceInfo& info, void* custom);

		static void AE_CALL enumDevices(void* custom, EnumDevicesCallback);

		static bool AE_CALL isValid(const HIDDeviceInfo& info);
		static bool AE_CALL isValid(const HIDDevice& device);

		static uint16_t AE_CALL getVendorID(const HIDDeviceInfo& info);
		static uint16_t AE_CALL getProductID(const HIDDeviceInfo& info);
		static std::wstring AE_CALL getManufacturerString(const HIDDeviceInfo& info);
		static std::wstring AE_CALL getProductString(const HIDDeviceInfo& info);
		static std::string_view AE_CALL getPath(const HIDDeviceInfo& info);
		static uint16_t AE_CALL getUsagePage(const HIDDeviceInfo& info);
		static uint16_t AE_CALL getUsage(const HIDDeviceInfo& info);

		static HIDDevice* AE_CALL open(const std::string_view& path);
		static void AE_CALL close(HIDDevice& device);

		static size_t AE_CALL read(HIDDevice& device, void* data, size_t dataLength, size_t timeout);

		inline static bool AE_CALL isReadSuccess(size_t rst) {
			return rst < READ_OUT_WAITTING;
		}
	};
}