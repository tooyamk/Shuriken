#include "Gamepad.h"
#include "Input.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::xinput {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info) :
		_index(((InternalGUID&)*info.guid.getData()).index - 1),
		_input(input),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info) {
		memset(&_state, 0, sizeof(_state));

		_setDeadZone(GamepadVirtualKeyCode::L_STICK, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadVirtualKeyCode::R_STICK, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadVirtualKeyCode::L_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadVirtualKeyCode::R_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
	}

	Gamepad::~Gamepad() {
	}

	IntrusivePtr<events::IEventDispatcher<DeviceEvent>> Gamepad::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& Gamepad::getInfo() const {
		return _info;
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				std::shared_lock lock(_mutex);

				switch ((GamepadVirtualKeyCode)code) {
				case GamepadVirtualKeyCode::L_STICK:
					return _getStick(_state.Gamepad.sThumbLX, _state.Gamepad.sThumbLY, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::R_STICK:
					return _getStick(_state.Gamepad.sThumbRX, _state.Gamepad.sThumbRY, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::L_TRIGGER:
					return _getTrigger(_state.Gamepad.bLeftTrigger, (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				case GamepadVirtualKeyCode::R_TRIGGER:
					return _getTrigger(_state.Gamepad.bRightTrigger, (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				case GamepadVirtualKeyCode::DPAD:
					((DeviceStateValue*)values)[0] = _translateDpad(_state.Gamepad.wButtons);
					return 1;
				case GamepadVirtualKeyCode::A:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
					return 1;
				case GamepadVirtualKeyCode::B:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
					return 1;
				case GamepadVirtualKeyCode::X:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
					return 1;
				case GamepadVirtualKeyCode::Y:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
					return 1;
				case GamepadVirtualKeyCode::L_SHOULDER:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
					return 1;
				case GamepadVirtualKeyCode::R_SHOULDER:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
					return 1;
				case GamepadVirtualKeyCode::BACK:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
					return 1;
				case GamepadVirtualKeyCode::START:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
					return 1;
				case GamepadVirtualKeyCode::L_THUMB:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
					return 1;
				case GamepadVirtualKeyCode::R_THUMB:
					((DeviceStateValue*)values)[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
					return 1;
				default:
					return 0;
				}
			}

			return 0;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				((DeviceStateValue*)values)[0] = _getDeadZone((GamepadVirtualKeyCode)code);

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
				_setDeadZone((GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				return 1;
			}

			return 0;
		}
		case DeviceStateType::VIBRATION:
		{
			if (values && count) {
				if (count < 2) {
					_setVibration(((DeviceStateValue*)values)[0], Math::ZERO<DeviceStateValue>);
					return 1;
				} else {
					_setVibration(((DeviceStateValue*)values)[0], ((DeviceStateValue*)values)[1]);
					return 2;
				}
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	void Gamepad::poll(bool dispatchEvent) {
		XINPUT_STATE state;
		if (XInputGetState(_index, &state) != ERROR_SUCCESS) return;

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(&_state, &state, sizeof(XINPUT_STATE));

			return;
		}

		DWORD oriBtns;

		auto& curPad = state.Gamepad;
		auto curBtns = curPad.wButtons;

		SHORT oriLStickX, oriLStickY, oriRStickX, oriRStickY;
		auto ls = false, rs = false;

		SHORT oriLT, oriRT;
		auto lt = false, rt = false;

		{
			std::scoped_lock lock(_mutex);

			auto& oriPad = _state.Gamepad;
			oriBtns = oriPad.wButtons;

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

			oriPad.wButtons = curPad.wButtons;
		}

		bool dpad =
			(oriBtns & XINPUT_GAMEPAD_DPAD_UP) != (curBtns & XINPUT_GAMEPAD_DPAD_UP) ||
			(oriBtns & XINPUT_GAMEPAD_DPAD_RIGHT) != (curBtns & XINPUT_GAMEPAD_DPAD_RIGHT) ||
			(oriBtns & XINPUT_GAMEPAD_DPAD_DOWN) != (curBtns & XINPUT_GAMEPAD_DPAD_DOWN) ||
			(oriBtns & XINPUT_GAMEPAD_DPAD_LEFT) != (curBtns & XINPUT_GAMEPAD_DPAD_LEFT);

		if (ls) _dispatchStick(oriLStickX, oriLStickY, curPad.sThumbLX, curPad.sThumbLY, GamepadVirtualKeyCode::L_STICK);
		if (rs) _dispatchStick(oriRStickX, oriRStickY, curPad.sThumbRX, curPad.sThumbRY, GamepadVirtualKeyCode::R_STICK);

		if (lt) _dispatchTrigger(oriLT, curPad.bLeftTrigger, GamepadVirtualKeyCode::L_TRIGGER);
		if (rt) _dispatchTrigger(oriRT, curPad.bRightTrigger, GamepadVirtualKeyCode::R_TRIGGER);

		if (dpad) {
			auto value = _translateDpad(curPad.wButtons);
			DeviceState k = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &value };
			_eventDispatcher->dispatchEvent(this, value >= Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}

		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_A, GamepadVirtualKeyCode::A);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_B, GamepadVirtualKeyCode::B);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_X, GamepadVirtualKeyCode::X);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_Y, GamepadVirtualKeyCode::Y);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_LEFT_SHOULDER, GamepadVirtualKeyCode::L_SHOULDER);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_RIGHT_SHOULDER, GamepadVirtualKeyCode::R_SHOULDER);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_BACK, GamepadVirtualKeyCode::BACK);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_START, GamepadVirtualKeyCode::START);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_LEFT_THUMB, GamepadVirtualKeyCode::L_THUMB);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_RIGHT_THUMB, GamepadVirtualKeyCode::R_THUMB);
	}

	void Gamepad::_setDeadZone(GamepadVirtualKeyCode keyCode, DeviceStateValue deadZone) {
		if (deadZone < Math::ZERO<DeviceStateValue>) deadZone = -deadZone;

		std::scoped_lock lock(_deadZoneMutex);

		_deadZone.insert_or_assign(keyCode, deadZone);
	}

	void Gamepad::_setVibration(DeviceStateValue left, DeviceStateValue right) {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = Math::clamp(left, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * NUMBER_65535<decltype(vibration.wLeftMotorSpeed)>;
		vibration.wRightMotorSpeed = Math::clamp(right, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * NUMBER_65535<decltype(vibration.wRightMotorSpeed)>;
		XInputSetState(_index, &vibration);
	}

	DeviceStateValue Gamepad::_translateDpad(WORD value) {
		switch (value & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT)) {
		case XINPUT_GAMEPAD_DPAD_UP:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::ZERO<DeviceStateValue>;
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI_4<DeviceStateValue>;
		case XINPUT_GAMEPAD_DPAD_RIGHT:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI_2<DeviceStateValue>;
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI<DeviceStateValue> - Math::PI_4<DeviceStateValue>;
		case XINPUT_GAMEPAD_DPAD_DOWN:
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<DeviceStateValue>;
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<DeviceStateValue> + Math::PI_4<DeviceStateValue>;
		case XINPUT_GAMEPAD_DPAD_LEFT:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<DeviceStateValue> + Math::PI_2<DeviceStateValue>;
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI2<DeviceStateValue> - Math::PI_4<DeviceStateValue>;
		default:
			return Math::NEGATIVE_ONE<DeviceStateValue>;
		}
	}

	DeviceState::CountType Gamepad::_getStick(SHORT x, SHORT y, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
		DeviceState::CountType c = 1;

		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;

		auto dx = _translateStick<false>(x);
		auto dy = _translateStick<true>(y);

		auto d2 = dx * dx + dy * dy;
		if (d2 > Math::ONE<DeviceStateValue>) d2 = Math::ONE<DeviceStateValue>;

		if (d2 <= dz2) {
			data[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
			if (count > 1) data[c++] = Math::ZERO<DeviceStateValue>;
		} else {
			data[0] = std::atan2(dy, dx) + Math::PI_2<DeviceStateValue>;
			if (data[0] < Math::ZERO<DeviceStateValue>) data[0] += Math::PI2<DeviceStateValue>;
			if (count > 1) data[c++] = _translateDeadZone01(d2 < Math::ONE<DeviceStateValue> ? std::sqrt(d2) : Math::ONE<DeviceStateValue>, dz, false);
		}

		return c;
	}

	DeviceState::CountType Gamepad::_getTrigger(SHORT t, GamepadVirtualKeyCode key, DeviceStateValue& data) const {
		auto dz = _getDeadZone(key);

		data = _translateTrigger(t);
		data = _translateDeadZone01(data, dz, data <= dz);

		return 1;
	}

	void Gamepad::_dispatchStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadVirtualKeyCode key) {
		DeviceStateValue value[] = { _translateStick<false>(curX) , _translateStick<true>(curY) };
		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;
		auto x = _translateStick<false>(oriX), y = _translateStick<true>(oriY);
		auto oriDz = x * x + y * y <= dz2;
		auto d2 = value[0] * value[0] + value[1] * value[1];
		if (d2 > Math::ONE<DeviceStateValue>) d2 = Math::ONE<DeviceStateValue>;
		auto curDz = d2 <= dz2;
		if (!oriDz || oriDz != curDz) {
			if (curDz) {
				value[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
				value[1] = Math::ZERO<DeviceStateValue>;
			} else {
				value[0] = std::atan2(value[1], value[0]) + Math::PI_2<float32_t>;
				if (value[0] < Math::ZERO<DeviceStateValue>) value[0] += Math::PI2<float32_t>;
				value[1] = _translateDeadZone01(d2 < Math::ONE<DeviceStateValue> ? std::sqrt(d2) : Math::ONE<DeviceStateValue>, dz, false);
			}
			DeviceState k = { (DeviceState::CodeType)key, 2, value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void Gamepad::_dispatchTrigger(SHORT ori, SHORT cur, GamepadVirtualKeyCode key) {
		auto value = _translateTrigger(cur);
		auto dz = _getDeadZone(key);
		auto oriDz = _translateTrigger(ori) <= dz;
		auto curDz = value <= dz;
		if (!curDz || oriDz != curDz) {
			value = _translateDeadZone01(value, dz, curDz);
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void Gamepad::_dispatchButton(WORD ori, WORD cur, uint16_t flags, GamepadVirtualKeyCode key) {
		auto curDown = cur & flags;
		if ((ori & flags) != curDown) {
			auto value = curDown ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher->dispatchEvent(this, curDown ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}
}