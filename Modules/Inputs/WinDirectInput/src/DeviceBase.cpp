#include "DeviceBase.h"
#include "DirectInput.h"

namespace aurora::modules::win_direct_input {
	DeviceBase::DeviceBase(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info) :
		_input(input->ref<DirectInput>()),
		_dev(dev),
		_info(info) {
	}

	DeviceBase::~DeviceBase() {
		_dev->Unacquire();
		_dev->Release();
		Ref::setNull(_input);
	}

	events::IEventDispatcher<InputDeviceEvent>& DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const InputDeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}
}