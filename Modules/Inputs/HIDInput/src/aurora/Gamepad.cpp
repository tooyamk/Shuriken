#include "Gamepad.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : DeviceBase(input, info, hid) {
	}

	Key::CountType Gamepad::getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const {
		return 0;
	}

	void Gamepad::_parse(StateBuffer state, size_t size) {

	}
}