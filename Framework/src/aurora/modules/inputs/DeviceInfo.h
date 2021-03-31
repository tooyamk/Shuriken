#pragma once

#include "aurora/GUID.h"

namespace aurora::modules::inputs {
	enum class DeviceType : uint8_t;


	using DeviceGUID = GUID<16>;

	class AE_FW_DLL DeviceInfo {
	public:
		DeviceInfo();
		DeviceInfo(uint16_t vendorID, uint16_t productID, const DeviceGUID& guid, DeviceType type);
		DeviceInfo(const DeviceInfo& value);
		DeviceInfo(DeviceInfo&& value);

		uint16_t vendorID;
		uint16_t productID;
		DeviceGUID guid;
		DeviceType type;

		inline DeviceInfo& AE_CALL operator=(const DeviceInfo& value) {
			guid = value.guid;
			type = value.type;

			return *this;
		}
		inline DeviceInfo& AE_CALL operator=(DeviceInfo&& value) noexcept {
			guid = std::move(value.guid);
			type = value.type;

			return *this;
		}
	};
}