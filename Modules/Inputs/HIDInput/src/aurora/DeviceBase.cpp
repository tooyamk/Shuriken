#include "DeviceBase.h"
#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	DeviceBase::DeviceBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) :
		_input(input),
		_info(info),
		_hid(&hid) {
	}

	DeviceBase::~DeviceBase() {
		using namespace aurora::extensions;

		HID::close(*_hid);
	}

	events::IEventDispatcher<DeviceEvent>& DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}

	void DeviceBase::poll(bool dispatchEvent) {
		using namespace aurora::extensions;

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			HID::read(*_hid, _state, _stateSize, 0);

			return;
		}
	}

	void DeviceBase::_init(uint8_t* state, size_t size) {
		_state = state;
		_stateSize = size;
	}
}