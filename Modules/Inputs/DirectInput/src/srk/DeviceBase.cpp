#include "DeviceBase.h"
#include "Input.h"

namespace srk::modules::inputs::direct_input {
	DeviceBase::DeviceBase(Input& input, srk_IDirectInputDevice* dev, const DeviceInfo& info) :
		_input(input),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_dev(dev) {
	}

	DeviceBase::~DeviceBase() {
		_dev->Unacquire();
		_dev->Release();
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}
}