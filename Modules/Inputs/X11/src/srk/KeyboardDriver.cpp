#include "KeyboardDriver.h"
#include "Input.h"
#include "srk/Printer.h"

namespace srk::modules::inputs::x11 {
	KeyboardDriver::KeyboardDriver(Input& input, windows::IWindow& win) :
		_listener(input, win, DeviceType::KEYBOARD, &KeyboardDriver::_callback, this),
		_changed(false) {
		_listener.start();
	}

	KeyboardDriver::~KeyboardDriver() {
		close();
	}

	KeyboardDriver* KeyboardDriver::create(Input& input, windows::IWindow& win) {
		return new KeyboardDriver(input, win);
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

	void KeyboardDriver::_callback(const XEvent& evt, void* target) {
		auto vk = VK_MAPPER[evt.xkey.keycode];
		if constexpr (Environment::IS_DEBUG) {
			if (!GenericKeyboardBuffer::isValid(vk)) {
				printaln(evt.xkey.keycode);
				return;
			}
		}

		((KeyboardDriver*)target)->_setKey(vk, evt.type == KeyPress);
	}
}