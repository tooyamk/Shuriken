#include "DeviceBase.h"
#include "Input.h"
#include "srk/Printer.h"

namespace srk::modules::inputs::evdev {
	DeviceBase::DeviceBase(Input& input, int32_t fd, const DeviceInfo& info) :
		_input(input),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_fd(fd) {
	}

	DeviceBase::~DeviceBase() {
		::close(_fd);
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}
}