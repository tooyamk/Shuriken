#include "DeviceInfo.h"
#include "aurora/modules/inputs/IInputModule.h"

namespace aurora::modules::inputs {
	DeviceInfo::DeviceInfo() :
		type(DeviceType::UNKNOWN) {
	}

	DeviceInfo::DeviceInfo(const DeviceGUID& guid, DeviceType type) :
		guid(guid),
		type(type) {
	}

	DeviceInfo::DeviceInfo(const DeviceInfo& value) :
		guid(value.guid),
		type(value.type) {
	}

	DeviceInfo::DeviceInfo(DeviceInfo&& value) :
		guid(std::move(value.guid)),
		type(value.type) {
	}
}