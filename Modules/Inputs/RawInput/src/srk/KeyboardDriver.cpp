#include "KeyboardDriver.h"
#include "Input.h"
#include "srk/Printer.h"
#include "srk/StringUtility.h"

namespace srk::modules::inputs::raw_input {
	KeyboardDriver::KeyboardDriver(Input& input, windows::IWindow& win, HANDLE handle) :
		_listener(input, win, handle, DeviceType::KEYBOARD, _callback, this),
		_changed(false) {
		_listener.start();
	}

	KeyboardDriver::~KeyboardDriver() {
		close();
	}

	KeyboardDriver* KeyboardDriver::create(Input& input, windows::IWindow& win, HANDLE handle) {
		return new KeyboardDriver(input, win, handle);
	}

	std::optional<bool> KeyboardDriver::readFromDevice(GenericKeyboardBuffer& buffer) const {
		auto expected = true;
		if (_changed.compare_exchange_strong(expected, false, std::memory_order::release, std::memory_order::relaxed)) {
			{
				std::scoped_lock lock(_lock);
				memcpy(buffer.data, _inputBuffer.data, sizeof(_inputBuffer.data));
			}

			return std::make_optional(true);
		}

		return std::make_optional(false);
	}

	void KeyboardDriver::close() {
		_listener.close();
	}

	void KeyboardDriver::_setKey(KeyboardVirtualKeyCode vk, bool pressed) {
		if (_inputBuffer.set(vk, pressed, _lock)) _changed.store(true);
	}

	void KeyboardDriver::_callback(const RAWINPUT& rawInput, void* target) {
		using namespace std::string_view_literals;

		auto driver = (KeyboardDriver*)target;
		auto& kb = rawInput.data.keyboard;

		auto vk = _getVirtualKey(kb);
		if constexpr (Environment::IS_DEBUG) {
			if (!GenericKeyboardBuffer::isValid(vk)) {
				printaln(kb.VKey, L"    "sv, kb.MakeCode, L"    "sv, kb.Flags, L"    "sv, StringUtility::toString(kb.Message, 16));
				return;
			}
		}

		switch (kb.Message) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			driver->_setKey(vk, true);
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			driver->_setKey(vk, false);
			break;
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