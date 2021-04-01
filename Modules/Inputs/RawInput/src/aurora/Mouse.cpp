#include "Mouse.h"

namespace aurora::modules::inputs::raw_input {
	Mouse::Mouse(Input& input, IApplication& app, const InternalDeviceInfo& info) : DeviceBase(input, app, info) {
	}

	uint32_t Mouse::getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const {
		return 0;
	}

	void Mouse::poll(bool dispatchEvent) {
	}

	void Mouse::_rawInput(const RAWINPUT& rawInput) {

	}
}