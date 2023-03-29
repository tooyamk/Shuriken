#include "InputListener.h"
#include "Input.h"

namespace srk::modules::inputs::raw_input {
	InputListener::InputListener(Input& input, windows::IWindow& win, HANDLE handle, DeviceType type, Callback callback, void* callbackTarget) :
		_input(input),
		_win(win),
		_handle(handle),
		_type(type),
		_listening(false),
		_callback(callback),
		_callbackTarget(callbackTarget),
		_rawIputHandler(&InputListener::_rawInputCallback, this) {
		_rawIputHandler.ref();
	}

	InputListener::~InputListener() {
		if (_listening) {
			_win->getEventDispatcher()->removeEventListener(windows::WindowEvent::RAW_INPUT, _rawIputHandler);
			_input->unregisterRawInputDevices(_type);
		}
	}

	void InputListener::start() {
		if (_listening) return;

		_win->getEventDispatcher()->addEventListener(windows::WindowEvent::RAW_INPUT, _rawIputHandler);
		_input->registerRawInputDevices(_type);
		_listening = true;
	}

	void InputListener::_rawInputCallback(events::Event<windows::WindowEvent>& e) {
		RAWINPUT rawInput;
		UINT dwSize = sizeof(rawInput);
		if (auto size = GetRawInputData((HRAWINPUT)(*(LPARAM*)e.getData()), RID_INPUT, &rawInput, &dwSize, sizeof(RAWINPUTHEADER)); size != -1) {
			if (rawInput.header.hDevice == _handle) _callback(rawInput, _callbackTarget);
		}
	}
}