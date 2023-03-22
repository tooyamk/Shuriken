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
		DeviceInfo(DeviceInfo&& value) noexcept;

		uint16_t vendorID;
		uint16_t productID;
		DeviceGUID guid;
		DeviceType type;
		DeviceFlag flags;
		std::string name;

		inline DeviceInfo& SRK_CALL operator=(const DeviceInfo& value) {
			vendorID = value.vendorID;
			productID = value.productID;
			guid = value.guid;
			type = value.type;
			flags = value.flags;
			name = value.name;

			return *this;
		}
		inline DeviceInfo& SRK_CALL operator=(DeviceInfo&& value) noexcept {
			vendorID = value.vendorID;
			productID = value.productID;
			guid = std::move(value.guid);
			type = value.type;
			flags = value.flags;
			name = std::move(value.name);

			return *this;
		}
	};
}