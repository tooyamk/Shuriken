#include "Gamepad.h"
#include "Input.h"
#include "math/Math.h"

namespace aurora::modules::win_xinput {
	Gamepad::Gamepad(Input* input, const InputDeviceInfo& info) :
		_input(input),
		_info(info) {
		memset(&_state, 0, sizeof(_state));

		setDeadZone((ui8)GamepadKeyCode::LEFT_STICK, .05f);
		setDeadZone((ui8)GamepadKeyCode::RIGHT_STICK, .05f);
		setDeadZone((ui8)GamepadKeyCode::LEFT_TRIGGER, .05f);
		setDeadZone((ui8)GamepadKeyCode::RIGHT_TRIGGER, .05f);
	}

	Gamepad::~Gamepad() {
	}

	events::IEventDispatcher<InputDeviceEvent>& Gamepad::getEventDispatcher() {
		return _eventDispatcher;
	}

	const InputDeviceInfo& Gamepad::getInfo() const {
		return _info;
	}

	ui32 Gamepad::getKeyState(ui32 keyCode, f32* data, ui32 count) const {
		return 0;
	}

	void Gamepad::poll(bool dispatchEvent) {
		auto index = ((GUID&)*_info.guid.getData()).index - 1;

		if (!dispatchEvent) {
			XInputGetState(index, &_state);
			return;
		}

		XINPUT_STATE state;
		if (XInputGetState(index, &state) == ERROR_SUCCESS) {
			auto& oriPad = _state.Gamepad;
			auto oriBtns = oriPad.wButtons;

			auto& curPad = state.Gamepad;
			auto curBtns = curPad.wButtons;

			SHORT oriLStickX, oriLStickY, oriRStickX, oriRStickY;
			auto ls = false, rs = false;
			if (oriPad.sThumbLX != curPad.sThumbLX || oriPad.sThumbLY != curPad.sThumbLY) {
				oriLStickX = oriPad.sThumbLX;
				oriLStickY = oriPad.sThumbLY;
				oriPad.sThumbLX = curPad.sThumbLX;
				oriPad.sThumbLY = curPad.sThumbLY;
				ls = true;
			}
			if (oriPad.sThumbRX != curPad.sThumbRX || oriPad.sThumbRY != curPad.sThumbRY) {
				oriRStickX = oriPad.sThumbRX;
				oriRStickY = oriPad.sThumbRY;
				oriPad.sThumbRX = curPad.sThumbRX;
				oriPad.sThumbRY = curPad.sThumbRY;
				rs = true;
			}

			if (ls) _updateStick(oriLStickX, oriLStickY, curPad.sThumbLX, curPad.sThumbLY, GamepadKeyCode::LEFT_STICK);
			if (rs) _updateStick(oriRStickX, oriRStickY, curPad.sThumbRX, curPad.sThumbRY, GamepadKeyCode::RIGHT_STICK);
		}
	}

	void Gamepad::setDeadZone(ui32 keyCode, f32 deadZone) {
		if (deadZone < 0.f) deadZone = -deadZone;

		if (auto itr = _deadZone.find(keyCode); itr == _deadZone.end()) {
			_deadZone.emplace(keyCode, deadZone);
		} else {
			itr->second = deadZone;
		}
	}

	f32 Gamepad::_translateStick(SHORT value, bool flip) {
		auto v = f32(value) / 32767.f;
		return flip ? -v : v;
	}

	void Gamepad::_updateStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key) {
		f32 value[] = { _translateStick(curX, false) , _translateStick(curY, true) };
		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;
		auto x = _translateStick(oriX, false), y = _translateStick(oriY, true);
		auto oriDz = x * x + y * y <= dz2;
		auto d2 = value[0] * value[0] + value[1] * value[1];
		if (d2 > 1.f) d2 = 1.f;
		auto curDz = d2 <= dz2;
		if (!oriDz || oriDz != curDz) {
			if (curDz) {
				value[0] = -1.f;
				value[1] = 0.f;
			} else {
				value[0] = std::atan2(value[1], value[0]) + Math::PI_2<f32>;
				if (value[0] < 0.f) value[0] += Math::PI2<f32>;
				value[1] = _translateDeadZone0_1(d2 < 1.f ? std::sqrt(d2) : 1.f, dz, false);
			}
			_eventDispatcher.dispatchEvent(this, InputDeviceEvent::MOVE, &InputKey({ (ui8)key, 2, value }));
		}
	}
}