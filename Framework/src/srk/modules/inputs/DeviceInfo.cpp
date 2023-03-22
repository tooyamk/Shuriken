#include "DeviceInfo.h"
#include "srk/modules/inputs/InputModule.h"

namespace srk::modules::inputs {
	DeviceInfo::DeviceInfo() :
		vendorID(0),
		productID(0),
		type(DeviceType::UNKNOWN),
		flags(DeviceFlag::NONE) {
	}

	DeviceInfo::DeviceInfo(uint16_t vendorID, uint16_t productID, const DeviceGUID& guid, DeviceType type, DeviceFlag flags, const std::string_view& name) :
		vendorID(vendorID),
		productID(productID),
		guid(guid),
		type(type),
		flags(flags),
		name(name) {
	}

	DeviceInfo::DeviceInfo(const DeviceInfo& value) :
		vendorID(value.vendorID),
		productID(value.productID),
		guid(value.guid),
		type(value.type),
		flags(value.flags),
		name(value.name) {
	}

	DeviceInfo::DeviceInfo(DeviceInfo&& value) noexcept :
		vendorID(value.vendorID),
		productID(value.productID),
		guid(std::move(value.guid)),
		type(value.type),
		flags(value.flags),
		name (std::move(value.name)) {
	}
}