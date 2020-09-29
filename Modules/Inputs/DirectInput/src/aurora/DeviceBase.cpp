#include "DeviceBase.h"
#include "Input.h"

namespace aurora::modules::inputs::direct_input {
	DeviceBase::DeviceBase(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info) :
		_input(input),
		_dev(dev),
		_info(info) {
	}

	DeviceBase::~DeviceBase() {
		_dev->Unacquire();
		_dev->Release();
	}

	events::IEventDispatcher<DeviceEvent>& DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}
}