#include "KeyboardDriver.h"
#include "Input.h"

namespace srk::modules::inputs::raw_input {
	KeyboardDriver::KeyboardDriver(Input& input, windows::IWindow& win, HANDLE handle) :
		_listener(input, win, handle, DeviceType::KEYBOARD, &KeyboardDriver::_callback, this),
		_changed(false) {
		memset(&_inputBuffer, 0, sizeof(_inputBuffer));
		_listener.start();
	}

	KeyboardDriver::~KeyboardDriver() {
	}

	KeyboardDriver* KeyboardDriver::create(Input& input, windows::IWindow& win, HANDLE handle) {
		return new KeyboardDriver(input, win, handle);
	}

	bool KeyboardDriver::readStateFromDevice(GenericKeyboard::Buffer& buffer) const {
		if (!_changed.load()) return false;

		std::scoped_lock lock(_lock);
		memcpy(buffer.data, _inputBuffer.data, sizeof(_inputBuffer.data));

		return true;
	}

	void KeyboardDriver::_callback(const RAWINPUT& rawInput, void* target) {
		auto driver = (KeyboardDriver*)target;

		auto& kb = rawInput.data.keyboard;
		switch (kb.Message) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			{
				std::scoped_lock lock(driver->_lock);
				driver->_inputBuffer.set(_getVirtualKey(kb), true);
			}
			driver->_changed.store(true);

			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			{
				std::scoped_lock lock(driver->_lock);
				driver->_inputBuffer.set(_getVirtualKey(kb), false);
			}
			driver->_changed.store(true);

			break;
		}
		default:
			break;
		}
	}

	KeyboardVirtualKeyCode KeyboardDriver::_getVirtualKey(const RAWKEYBOARD& raw) {
		switch ((KeyboardVirtualKeyCode)raw.VKey) {
		case KeyboardVirtualKeyCode::SHIFT:
			return raw.MakeCode == 0x2A ? KeyboardVirtualKeyCode::L_SHIFT : KeyboardVirtualKeyCode::R_SHIFT;
		case KeyboardVirtualKeyCode::CTRL:
			return raw.Flags & RI_KEY_E0 ? KeyboardVirtualKeyCode::R_CTRL : KeyboardVirtualKeyCode::L_CTRL;
		case KeyboardVirtualKeyCode::ALT:
			return raw.Flags & RI_KEY_E0 ? KeyboardVirtualKeyCode::R_ALT : KeyboardVirtualKeyCode::L_ALT;
		default:
			return (KeyboardVirtualKeyCode)raw.VKey;
		}
	}
}