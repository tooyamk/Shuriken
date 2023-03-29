#include "DeviceBase.h"
#include "Input.h"

namespace srk::modules::inputs::raw_input {
	DeviceBase::DeviceBase(Input& input, windows::IWindow& win, const InternalDeviceInfo& info) :
		_listener(input, win, info.hDevice, info.type, &DeviceBase::_rawInputCallback, this),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info) {
	}

	DeviceBase::~DeviceBase() {
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}

	void DeviceBase::_rawInputCallback(const RAWINPUT& rawInput, void* target) {
		((DeviceBase*)target)->_rawInput(rawInput);
	}
}