#if AE_OS == AE_OS_WIN
#include "Gamepad.h"

namespace aurora::modules::inputs::generic_input {
	Gamepad::Gamepad(Input& input, const InternalDeviceInfo& info) : DeviceBase(input, info) {
	}

	uint32_t Gamepad::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		return 0;
	}

	void Gamepad::poll(bool dispatchEvent) {
		_read();
	}

	void Gamepad::_parse() {
		int a = 1;
	}
}
#endif