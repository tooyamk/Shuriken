#pragma once

#include "srk/GUID.h"

namespace srk::modules::inputs {
	enum class DeviceType : uint8_t;
	enum class DeviceFlag : uint8_t;


	using DeviceGUID = GUID<16>;

	class SRK_FW_DLL DeviceInfo {
	public:
		DeviceInfo();
		DeviceInfo(uint16_t vendorID, uint16_t productID, const DeviceGUID& guid, DeviceType type, DeviceFlag flags, const std::string_view& name);
		DeviceInfo(const DeviceInfo& value);
		DeviceInfo(DeviceInfo&& value);

		uint16_t vendorID;
		uint16_t productID;
		DeviceGUID guid;
		DeviceType type;
		DeviceFlag flags;
		std::string name;

		inline DeviceInfo& SRK_CALL operator=(const DeviceInfo& value) {
			guid = value.guid;
			type = value.type;

			return *this;
		}
		inline DeviceInfo& SRK_CALL operator=(DeviceInfo&& value) noexcept {
			guid = std::move(value.guid);
			type = value.type;

			return *this;
		}
	};
}