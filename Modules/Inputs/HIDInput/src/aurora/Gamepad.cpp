#include "Gamepad.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid) {
	}

	Key::CountType Gamepad::getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const {
		return 0;
	}

	void Gamepad::_parse(bool dispatchEvent, ReadBuffer& readBuffer, size_t readBufferSize) {
		/*
		auto first = state[0];
		auto buf = state + 1;

		auto a9 = buf[9];
		auto a10 = buf[10];
		auto a11 = buf[11];

		auto btnSrcVal = buf[10];
		auto square = (btnSrcVal & 0b10000) ? 0xFF : 0;
		auto tri = (btnSrcVal & 0b10000000) ? 0xFF : 0;
		if (square != 0) {
			//printdln(btnVal);
		}
		*/
		//if (btnSrcVal != 128) printdln(btnSrcVal);
		//printdln(String::toString(state, 16));

		int a = 1;
	}
}