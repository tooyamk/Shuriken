#if AE_OS == AE_OS_WIN
#include "Keyboard.h"

namespace aurora::modules::inputs::generic_input {
	Keyboard::Keyboard(Input& input, const InternalDeviceInfo& info) : DeviceBase(input, info) {
	}

	uint32_t Keyboard::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		return 0;
	}

	void Keyboard::poll(bool dispatchEvent) {
		_read();
	}

	void Keyboard::_parse() {
		int a = 1;
	}
}
#endif