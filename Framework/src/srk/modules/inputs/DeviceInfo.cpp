#include "DeviceInfo.h"
#include "srk/modules/inputs/InputModule.h"

namespace srk::modules::inputs {
	DeviceInfo::DeviceInfo() :
		vendorID(0),
		productID(0),
		type(DeviceType::UNKNOWN) {
	}

	DeviceInfo::DeviceInfo(uint16_t vendorID, uint16_t productID, const DeviceGUID& guid, DeviceType type) :
		vendorID(vendorID),
		productID(productID),
		guid(guid),
		type(type) {
	}

	DeviceInfo::DeviceInfo(const DeviceInfo& value) :
		vendorID(value.vendorID),
		productID(value.productID),
		guid(value.guid),
		type(value.type) {
	}

	DeviceInfo::DeviceInfo(DeviceInfo&& value) :
		vendorID(value.vendorID),
		productID(value.productID),
		guid(std::move(value.guid)),
		type(value.type) {
	}
}