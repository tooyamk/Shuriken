#pragma once

#include "modules/inputs/GUID.h"

namespace aurora::modules::inputs {
	enum class DeviceType : uint8_t;


	class AE_DLL DeviceInfo {
	public:
		DeviceInfo();
		DeviceInfo(const GUID& guid, DeviceType type);
		DeviceInfo(const DeviceInfo& value);
		DeviceInfo(DeviceInfo&& value);

		GUID guid;
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