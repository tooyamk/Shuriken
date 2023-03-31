#include "KeyboardDriver.h"
#include "Input.h"
//#include "srk/Printer.h"

namespace srk::modules::inputs::raw_input {
	KeyboardDriver::KeyboardDriver(Input& input, windows::IWindow& win, HANDLE handle) :
		_listener(input, win, handle, DeviceType::KEYBOARD, &KeyboardDriver::_callback, this),
		_changed(false) {
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
		using namespace std::string_view_literals;

		auto driver = (KeyboardDriver*)target;

		auto& kb = rawInput.data.keyboard;
		auto vk = _getVirtualKey(kb);
		//printaln(kb.VKey, "    "sv, kb.MakeCode, "    "sv, kb.Flags);
		if (!GenericKeyboard::Buffer::isValid(vk)) return;

		switch (kb.Message) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			{
				std::scoped_lock lock(driver->_lock);
				driver->_inputBuffer.set(vk, true);
			}
			driver->_changed.store(true);

			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			{
				std::scoped_lock lock(driver->_lock);
				driver->_inputBuffer.set(vk, false);
			}
			driver->_changed.store(true);

			break;
		}
		default:
			break;
		}
	}

	KeyboardVirtualKeyCode KeyboardDriver::_getVirtualKey(const RAWKEYBOARD& raw) {
		switch (raw.VKey) {
		case VK_SHIFT:
			return raw.MakeCode == 0x2A ? KeyboardVirtualKeyCode::L_SHIFT : KeyboardVirtualKeyCode::R_SHIFT;
		case VK_CONTROL:
			return raw.Flags & RI_KEY_E0 ? KeyboardVirtualKeyCode::R_CONTROL : KeyboardVirtualKeyCode::L_CONTROL;
		case VK_MENU:
			return raw.Flags & RI_KEY_E0 ? KeyboardVirtualKeyCode::R_ALT : KeyboardVirtualKeyCode::L_ALT;
		case VK_RETURN:
			return raw.Flags & RI_KEY_E0 ? KeyboardVirtualKeyCode::NUMPAD_ENTER : KeyboardVirtualKeyCode::ENTER;
		default:
			return VK_MAPPER[raw.VKey];
		}
	}
}