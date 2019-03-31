#include "DeviceBase.h"

namespace aurora::modules::win_direct_input {
	DeviceBase::DeviceBase(LPDIRECTINPUTDEVICE8 dev, const InputDeviceGUID& guid, ui32 type) :
		_dev(dev),
		_guid(guid),
		_type(type) {
	}

	const InputDeviceGUID& DeviceBase::getGUID() const {
		return _guid;
	}

	ui32 DeviceBase::getType() const {
		return _type;
	}
}