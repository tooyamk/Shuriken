#include "Gamepad.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid) {
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				((DeviceStateValue*)values)[0] = _getDeadZone((GamepadKeyCode)code);

				return 1;
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				_setDeadZone((GamepadKeyCode)code, ((DeviceStateValue*)values)[0]);
				return 1;
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	void Gamepad::_doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) {
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
		printdln(String::toString(inputBuffer, 16));

		//028101810080FB7D0080 00 000000CCCC
		//0281FF7F0080F97C0080 01 000000CCCC a
		//028101810080F97C0080 02 000000CCCC b
		//048201810080FB7D0080 04 000000CCCC x
		//02810181FE7EF97C0080 08 000000CCCC y

		int a = 1;
	}

	bool Gamepad::_doOutput() {
		return false;
	}
}