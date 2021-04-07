#include "Gamepad.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info) : DeviceBase(input, info) {
	}

	uint32_t Gamepad::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		return 0;
	}

	void Gamepad::poll(bool dispatchEvent) {
		//_read();
	}

	void Gamepad::_parse() {
		int a = 1;
	}
}