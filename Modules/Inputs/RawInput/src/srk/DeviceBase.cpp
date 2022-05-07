#include "DeviceBase.h"
#include "Input.h"
#include "srk/Debug.h"

namespace srk::modules::inputs::raw_input {
	DeviceBase::DeviceBase(Input& input, IApplication& app, const InternalDeviceInfo& info) :
		_input(input),
		_app(app),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_rawIputHandler(events::createEventListener(&DeviceBase::_rawInputCallback, this)) {
		_app->getEventDispatcher()->addEventListener(ApplicationEvent::RAW_INPUT, _rawIputHandler);
	}

	DeviceBase::~DeviceBase() {
		if (_rawIputHandler) _app->getEventDispatcher()->removeEventListener(ApplicationEvent::RAW_INPUT, _rawIputHandler);
		_input->unregisterRawInputDevices(_info.type);
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}

	void DeviceBase::_rawInputCallback(events::Event<ApplicationEvent>& e) {
		using namespace std::literals;

		BYTE buf[128];
		UINT dwSize = sizeof(buf);
		if (auto size = GetRawInputData((HRAWINPUT)(*(LPARAM*)e.getData()), RID_INPUT, buf, &dwSize, sizeof(RAWINPUTHEADER)); size != -1) {
			auto& rawInput = *(RAWINPUT*)buf;
			if (rawInput.header.hDevice == _info.hDevice) _rawInput(rawInput);
		} else {
			printaln(L"RawInputModule rawInputCallback error"sv);
		}
	}
}