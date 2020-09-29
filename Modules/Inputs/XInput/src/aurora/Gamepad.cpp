#include "Gamepad.h"
#include "Input.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::win_xinput {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info) :
		_index(((InternalGUID&)*info.guid.getData()).index - 1),
		_input(input),
		_info(info) {
		memset(&_state, 0, sizeof(_state));

		setDeadZone((uint8_t)GamepadKeyCode::LEFT_STICK, .05f);
		setDeadZone((uint8_t)GamepadKeyCode::RIGHT_STICK, .05f);
		setDeadZone((uint8_t)GamepadKeyCode::LEFT_TRIGGER, .05f);
		setDeadZone((uint8_t)GamepadKeyCode::RIGHT_TRIGGER, .05f);
	}

	Gamepad::~Gamepad() {
	}

	events::IEventDispatcher<DeviceEvent>& Gamepad::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& Gamepad::getInfo() const {
		return _info;
	}

	uint32_t Gamepad::getKeyState (uint32_t keyCode, float32_t* data, uint32_t count) const {
		if (data && count) {
			switch ((GamepadKeyCode)keyCode) {
			case GamepadKeyCode::LEFT_STICK:
				return _getStick(_state.Gamepad.sThumbLX, _state.Gamepad.sThumbLY, (GamepadKeyCode)keyCode, data, count);
			case GamepadKeyCode::RIGHT_STICK:
				return _getStick(_state.Gamepad.sThumbRX, _state.Gamepad.sThumbRY, (GamepadKeyCode)keyCode, data, count);
			case GamepadKeyCode::LEFT_TRIGGER:
				return _getTrigger(_state.Gamepad.bLeftTrigger, (GamepadKeyCode)keyCode, data[0]);
			case GamepadKeyCode::RIGHT_TRIGGER:
				return _getTrigger(_state.Gamepad.bRightTrigger, (GamepadKeyCode)keyCode, data[0]);
			case GamepadKeyCode::DPAD:
				data[0] = _translateDpad(_state.Gamepad.wButtons);
				return 1;
			case GamepadKeyCode::A:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
				return 1;
			case GamepadKeyCode::B:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
				return 1;
			case GamepadKeyCode::X:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
				return 1;
			case GamepadKeyCode::Y:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
				return 1;
			case GamepadKeyCode::LEFT_SHOULDER:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
				return 1;
			case GamepadKeyCode::RIGHT_SHOULDER:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
				return 1;
			case GamepadKeyCode::BACK:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
				return 1;
			case GamepadKeyCode::START:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
				return 1;
			case GamepadKeyCode::LEFT_THUMB:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
				return 1;
			case GamepadKeyCode::RIGHT_THUMB:
				data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
				return 1;
			default:
				return 0;
			}
		}
		return 0;
	}

	void Gamepad::poll(bool dispatchEvent) {
		if (!dispatchEvent) {
			XInputGetState(_index, &_state);
			return;
		}

		XINPUT_STATE state;
		if (XInputGetState(_index, &state) == ERROR_SUCCESS) {
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

			auto lt = false, rt = false;
			SHORT oriLT, oriRT;
			if (oriPad.bLeftTrigger != curPad.bLeftTrigger) {
				oriLT = oriPad.bLeftTrigger;
				oriPad.bLeftTrigger = curPad.bLeftTrigger;
				lt = true;
			}
			if (oriPad.bRightTrigger != curPad.bRightTrigger) {
				oriRT = oriPad.bRightTrigger;
				oriPad.bRightTrigger = curPad.bRightTrigger;
				rt = true;
			}

			auto dpad = 
				(oriBtns & XINPUT_GAMEPAD_DPAD_UP) != (curBtns & XINPUT_GAMEPAD_DPAD_UP) ||
				(oriBtns & XINPUT_GAMEPAD_DPAD_RIGHT) != (curBtns & XINPUT_GAMEPAD_DPAD_RIGHT) ||
				(oriBtns & XINPUT_GAMEPAD_DPAD_DOWN) != (curBtns & XINPUT_GAMEPAD_DPAD_DOWN) ||
				(oriBtns & XINPUT_GAMEPAD_DPAD_LEFT) != (curBtns & XINPUT_GAMEPAD_DPAD_LEFT);

			oriPad.wButtons = curPad.wButtons;

			if (ls) _updateStick(oriLStickX, oriLStickY, curPad.sThumbLX, curPad.sThumbLY, GamepadKeyCode::LEFT_STICK);
			if (rs) _updateStick(oriRStickX, oriRStickY, curPad.sThumbRX, curPad.sThumbRY, GamepadKeyCode::RIGHT_STICK);

			if (lt) _updateTrigger(oriLT, curPad.bLeftTrigger, GamepadKeyCode::LEFT_TRIGGER);
			if (rt) _updateTrigger(oriRT, curPad.bRightTrigger, GamepadKeyCode::RIGHT_TRIGGER);

			if (dpad) {
				float32_t value = _translateDpad(curPad.wButtons);
				_eventDispatcher.dispatchEvent(this, value >= 0.f ? DeviceEvent::DOWN : DeviceEvent::UP, &Key({ (uint8_t)GamepadKeyCode::DPAD, 1, &value }));
			}

			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_A, GamepadKeyCode::A);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_B, GamepadKeyCode::B);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_X, GamepadKeyCode::X);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_Y, GamepadKeyCode::Y);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_LEFT_SHOULDER, GamepadKeyCode::LEFT_SHOULDER);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_RIGHT_SHOULDER, GamepadKeyCode::RIGHT_SHOULDER);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_BACK, GamepadKeyCode::BACK);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_START, GamepadKeyCode::START);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_LEFT_THUMB, GamepadKeyCode::LEFT_THUMB);
			_updateButton(oriBtns, curBtns, XINPUT_GAMEPAD_RIGHT_THUMB, GamepadKeyCode::RIGHT_THUMB);
		}
	}

	void Gamepad::setDeadZone (uint32_t keyCode, float32_t deadZone) {
		if (deadZone < 0.f) deadZone = -deadZone;

		_deadZone.insert_or_assign(keyCode, deadZone);
	}

	void Gamepad::setVibration(float32_t left, float32_t right) {
		_vibration.wLeftMotorSpeed = Math::clamp(left, 0.f, 1.f) * 65535;
		_vibration.wRightMotorSpeed = Math::clamp(right, 0.f, 1.f) * 65535;
		XInputSetState(_index, &_vibration);
	}

	float32_t Gamepad::_translateDpad(WORD value) {
		switch (value & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT)) {
		case XINPUT_GAMEPAD_DPAD_UP:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_LEFT:
			return 0.f;
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI_4<float32_t>;
		case XINPUT_GAMEPAD_DPAD_RIGHT:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI_2<float32_t>;
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI<float32_t> - Math::PI_4<float32_t>;
		case XINPUT_GAMEPAD_DPAD_DOWN:
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<float32_t>;
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<float32_t> + Math::PI_4<float32_t>;
		case XINPUT_GAMEPAD_DPAD_LEFT:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<float32_t> + Math::PI_2<float32_t>;
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI2<float32_t> - Math::PI_4<float32_t>;
		default:
			return -1.f;
		}
	}

	uint32_t Gamepad::_getStick(SHORT x, SHORT y, GamepadKeyCode key, float32_t* data, uint32_t count) const {
		uint32_t c = 1;

		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;

		auto dx = _translateStick<false>(x);
		auto dy = _translateStick<true>(y);

		auto d2 = dx * dx + dy * dy;
		if (d2 > 1.f) d2 = 1.f;

		if (d2 <= dz2) {
			data[0] = -1.0f;
			if (count > 1) data[c++] = 0.f;
		} else {
			data[0] = std::atan2(dy, dx) + Math::PI_2<float32_t>;
			if (data[0] < 0.f) data[0] += Math::PI2<float32_t>;
			if (count > 1) data[c++] = _translateDeadZone0_1(d2 < 1.f ? std::sqrt(d2) : 1.f, dz, false);
		}

		return c;
	}

	uint32_t Gamepad::_getTrigger(SHORT t, GamepadKeyCode key, float32_t& data) const {
		auto dz = _getDeadZone(key);

		data = _translateTrigger(t);
		data = _translateDeadZone0_1(data, dz, data <= dz);

		return 1;
	}

	void Gamepad::_updateStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key) {
		float32_t value[] = { _translateStick<false>(curX) , _translateStick<true>(curY) };
		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;
		auto x = _translateStick<false>(oriX), y = _translateStick<true>(oriY);
		auto oriDz = x * x + y * y <= dz2;
		auto d2 = value[0] * value[0] + value[1] * value[1];
		if (d2 > 1.f) d2 = 1.f;
		auto curDz = d2 <= dz2;
		if (!oriDz || oriDz != curDz) {
			if (curDz) {
				value[0] = -1.f;
				value[1] = 0.f;
			} else {
				value[0] = std::atan2(value[1], value[0]) + Math::PI_2<float32_t>;
				if (value[0] < 0.f) value[0] += Math::PI2<float32_t>;
				value[1] = _translateDeadZone0_1(d2 < 1.f ? std::sqrt(d2) : 1.f, dz, false);
			}
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &Key({ (uint8_t)key, 2, value }));
		}
	}

	void Gamepad::_updateTrigger(SHORT ori, SHORT cur, GamepadKeyCode key) {
		float32_t value = _translateTrigger(cur);
		auto dz = _getDeadZone(key);
		auto oriDz = _translateTrigger(ori) <= dz;
		auto curDz = value <= dz;
		ori = cur;
		if (!curDz || oriDz != curDz) {
			value = _translateDeadZone0_1(value, dz, curDz);
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &Key({ (uint8_t)key, 1, &value }));
		}
	}

	void Gamepad::_updateButton(WORD ori, WORD cur, uint16_t flags, GamepadKeyCode key) {
		auto curDown = cur & flags;
		if ((ori & flags) != curDown) {
			float32_t value = curDown ? 1.f : 0.f;
			_eventDispatcher.dispatchEvent(this, curDown ? DeviceEvent::DOWN : DeviceEvent::UP, &Key({ (uint8_t)key, 1, &value }));
		}
	}
}