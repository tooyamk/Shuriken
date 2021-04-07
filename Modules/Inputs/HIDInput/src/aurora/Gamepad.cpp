#include "Gamepad.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : DeviceBase(input, info, hid) {
		_init(_state, sizeof(_state));
	}

	uint32_t Gamepad::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		return 0;
	}
}