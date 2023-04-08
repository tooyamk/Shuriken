#include "InputListener.h"
#include "Input.h"

namespace srk::modules::inputs::x11 {
	InputListener::InputListener(Input& input, windows::IWindow& win, DeviceType type, Callback callback, void* callbackTarget) :
		_input(input),
		_win(win),
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
		_listening = true;
	}

	void InputListener::close() {
		std::scoped_lock lock(_lock);

		if (!_listening) return;
		
		_win->getEventDispatcher()->removeEventListener(windows::WindowEvent::INPUT, _iputHandler);
	}

	void InputListener::_inputCallback(events::Event<windows::WindowEvent>& e) {
		auto evt = (XEvent*)e.getData();

		switch (evt->type) {
		case KeyPress:
		case KeyRelease:
			if (_type == DeviceType::KEYBOARD) _callback(*evt, _callbackTarget);
			break;
		case ButtonPress:
		case ButtonRelease:
		case MotionNotify:
			if (_type == DeviceType::MOUSE) _callback(*evt, _callbackTarget);
			break;
		default:
			break;
		}
	}
}