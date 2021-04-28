#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	DeviceBase::DeviceBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) :
		_input(input),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_hid(&hid) {
	}

	DeviceBase::~DeviceBase() {
		using namespace aurora::extensions;

		HID::close(*_hid);
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}
}