#include "DeviceBase.h"
#include "Input.h"
#include "srk/Debug.h"

namespace srk::modules::inputs::raw_input {
	DeviceBase::DeviceBase(Input& input, windows::IWindow& win, const InternalDeviceInfo& info) :
		_input(input),
		_win(win),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info),
		_rawIputHandler(events::createEventListener(&DeviceBase::_rawInputCallback, this)) {
		_win->getEventDispatcher()->addEventListener(windows::WindowEvent::RAW_INPUT, _rawIputHandler);
	}

	DeviceBase::~DeviceBase() {
		if (_rawIputHandler) _win->getEventDispatcher()->removeEventListener(windows::WindowEvent::RAW_INPUT, _rawIputHandler);
		_input->unregisterRawInputDevices(_info.type);
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}

	void DeviceBase::_rawInputCallback(events::Event<windows::WindowEvent>& e) {
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