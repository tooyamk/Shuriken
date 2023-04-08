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
		_iputHandler(&InputListener::_inputCallback, this) {
		_iputHandler.ref();
	}

	InputListener::~InputListener() {
		close();
	}

	void InputListener::start() {
		std::scoped_lock lock(_lock);

		if (_listening) return;

		_win->getEventDispatcher()->addEventListener(windows::WindowEvent::INPUT, _iputHandler);
		_input->registerRawInputDevices(_type);
		_listening = true;
	}

	void InputListener::close() {
		std::scoped_lock lock(_lock);

		if (!_listening) return;
		
		_win->getEventDispatcher()->removeEventListener(windows::WindowEvent::INPUT, _iputHandler);
		_input->unregisterRawInputDevices(_type);
	}

	HWND InputListener::getHWND() const {
		return _input->getHWND();
	}

	void InputListener::_inputCallback(events::Event<windows::WindowEvent>& e) {
		auto hRawInput = (HRAWINPUT)(*(LPARAM*)e.getData());

		UINT bufferSize;
		if (GetRawInputData(hRawInput, RID_INPUT, nullptr, &bufferSize, sizeof(RAWINPUTHEADER)) == (UINT)-1) return;

		constexpr size_t stackMemSize = 128;
		if (bufferSize <= stackMemSize) {
			uint8_t stackMem[stackMemSize];
			_doRawInput(hRawInput, stackMem, stackMemSize);
		} else {
			if (auto mem = malloc(bufferSize); mem) {
				_doRawInput(hRawInput, mem, bufferSize);
				free(mem);
			}
		}
	}

	void InputListener::_doRawInput(HRAWINPUT ri, void* mem, UINT memSize) {
		auto rawInput = (PRAWINPUT)mem;
		if (auto size = GetRawInputData(ri, RID_INPUT, rawInput, &memSize, sizeof(RAWINPUTHEADER)); size != (UINT)-1) {
			if (!rawInput->header.hDevice || rawInput->header.hDevice == _handle) _callback(*rawInput, _callbackTarget);
		}
	}
}