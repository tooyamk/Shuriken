#include "Gamepad.h"
#include "Input.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::xinput {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info) :
		_index(((InternalGUID&)*info.guid.getData()).index - 1),
		_input(input),
		_info(info) {
		memset(&_state, 0, sizeof(_state));

		_setDeadZone(GamepadKeyCode::L_STICK, Math::TWENTIETH<DeviceState::ValueType>);
		_setDeadZone(GamepadKeyCode::R_STICK, Math::TWENTIETH<DeviceState::ValueType>);
		_setDeadZone(GamepadKeyCode::L_TRIGGER, Math::TWENTIETH<DeviceState::ValueType>);
		_setDeadZone(GamepadKeyCode::R_TRIGGER, Math::TWENTIETH<DeviceState::ValueType>);
	}

	Gamepad::~Gamepad() {
	}

	events::IEventDispatcher<DeviceEvent>& Gamepad::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& Gamepad::getInfo() const {
		return _info;
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (data && count) {
				std::shared_lock lock(_mutex);

				switch ((GamepadKeyCode)code) {
				case GamepadKeyCode::L_STICK:
					return _getStick(_state.Gamepad.sThumbLX, _state.Gamepad.sThumbLY, (GamepadKeyCode)code, data, count);
				case GamepadKeyCode::R_STICK:
					return _getStick(_state.Gamepad.sThumbRX, _state.Gamepad.sThumbRY, (GamepadKeyCode)code, data, count);
				case GamepadKeyCode::L_TRIGGER:
					return _getTrigger(_state.Gamepad.bLeftTrigger, (GamepadKeyCode)code, data[0]);
				case GamepadKeyCode::R_TRIGGER:
					return _getTrigger(_state.Gamepad.bRightTrigger, (GamepadKeyCode)code, data[0]);
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
				case GamepadKeyCode::L_SHOULDER:
					data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
					return 1;
				case GamepadKeyCode::R_SHOULDER:
					data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
					return 1;
				case GamepadKeyCode::BACK:
					data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
					return 1;
				case GamepadKeyCode::START:
					data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
					return 1;
				case GamepadKeyCode::L_THUMB:
					data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
					return 1;
				case GamepadKeyCode::R_THUMB:
					data[0] = _translateButton(_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
					return 1;
				default:
					return 0;
				}
			}

			return 0;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (data && count) {
				data[0] = _getDeadZone((GamepadKeyCode)code);

				return 1;
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::DEAD_ZONE:
		{
			if (data && count) {
				_setDeadZone((GamepadKeyCode)code, data[0]);
				return 1;
			}

			return 0;
		}
		case DeviceStateType::VIBRATION:
		{
			if (data && count) {
				if (count < 2) {
					_setVibration(data[0], Math::ZERO<DeviceState::ValueType>);
					return 1;
				} else {
					_setVibration(data[0], data[1]);
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

		if (ls) _dispatchStick(oriLStickX, oriLStickY, curPad.sThumbLX, curPad.sThumbLY, GamepadKeyCode::L_STICK);
		if (rs) _dispatchStick(oriRStickX, oriRStickY, curPad.sThumbRX, curPad.sThumbRY, GamepadKeyCode::R_STICK);

		if (lt) _dispatchTrigger(oriLT, curPad.bLeftTrigger, GamepadKeyCode::L_TRIGGER);
		if (rt) _dispatchTrigger(oriRT, curPad.bRightTrigger, GamepadKeyCode::R_TRIGGER);

		if (dpad) {
			auto value = _translateDpad(curPad.wButtons);
			DeviceState k = { (DeviceState::CodeType)GamepadKeyCode::DPAD, 1, &value };
			_eventDispatcher.dispatchEvent(this, value >= Math::ZERO<DeviceState::ValueType> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}

		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_A, GamepadKeyCode::A);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_B, GamepadKeyCode::B);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_X, GamepadKeyCode::X);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_Y, GamepadKeyCode::Y);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_LEFT_SHOULDER, GamepadKeyCode::L_SHOULDER);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_RIGHT_SHOULDER, GamepadKeyCode::R_SHOULDER);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_BACK, GamepadKeyCode::BACK);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_START, GamepadKeyCode::START);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_LEFT_THUMB, GamepadKeyCode::L_THUMB);
		_dispatchButton(oriBtns, curBtns, XINPUT_GAMEPAD_RIGHT_THUMB, GamepadKeyCode::R_THUMB);
	}

	void Gamepad::_setDeadZone(GamepadKeyCode keyCode, DeviceState::ValueType deadZone) {
		if (deadZone < Math::ZERO<DeviceState::ValueType>) deadZone = -deadZone;

		std::scoped_lock lock(_deadZoneMutex);

		_deadZone.insert_or_assign(keyCode, deadZone);
	}

	void Gamepad::_setVibration(DeviceState::ValueType left, DeviceState::ValueType right) {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = Math::clamp(left, Math::ZERO<DeviceState::ValueType>, Math::ONE<DeviceState::ValueType>) * NUMBER_65535<decltype(vibration.wLeftMotorSpeed)>;
		vibration.wRightMotorSpeed = Math::clamp(right, Math::ZERO<DeviceState::ValueType>, Math::ONE<DeviceState::ValueType>) * NUMBER_65535<decltype(vibration.wRightMotorSpeed)>;
		XInputSetState(_index, &vibration);
	}

	DeviceState::ValueType Gamepad::_translateDpad(WORD value) {
		switch (value & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT)) {
		case XINPUT_GAMEPAD_DPAD_UP:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::ZERO<DeviceState::ValueType>;
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI_4<DeviceState::ValueType>;
		case XINPUT_GAMEPAD_DPAD_RIGHT:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI_2<DeviceState::ValueType>;
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
			return Math::PI<DeviceState::ValueType> - Math::PI_4<DeviceState::ValueType>;
		case XINPUT_GAMEPAD_DPAD_DOWN:
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<DeviceState::ValueType>;
		case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<DeviceState::ValueType> + Math::PI_4<DeviceState::ValueType>;
		case XINPUT_GAMEPAD_DPAD_LEFT:
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI<DeviceState::ValueType> + Math::PI_2<DeviceState::ValueType>;
		case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT:
			return Math::PI2<DeviceState::ValueType> - Math::PI_4<DeviceState::ValueType>;
		default:
			return Math::NEGATIVE_ONE<DeviceState::ValueType>;
		}
	}

	DeviceState::CountType Gamepad::_getStick(SHORT x, SHORT y, GamepadKeyCode key, DeviceState::ValueType* data, DeviceState::CountType count) const {
		DeviceState::CountType c = 1;

		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;

		auto dx = _translateStick<false>(x);
		auto dy = _translateStick<true>(y);

		auto d2 = dx * dx + dy * dy;
		if (d2 > Math::ONE<DeviceState::ValueType>) d2 = Math::ONE<DeviceState::ValueType>;

		if (d2 <= dz2) {
			data[0] = Math::NEGATIVE_ONE<DeviceState::ValueType>;
			if (count > 1) data[c++] = Math::ZERO<DeviceState::ValueType>;
		} else {
			data[0] = std::atan2(dy, dx) + Math::PI_2<DeviceState::ValueType>;
			if (data[0] < Math::ZERO<DeviceState::ValueType>) data[0] += Math::PI2<DeviceState::ValueType>;
			if (count > 1) data[c++] = _translateDeadZone01(d2 < Math::ONE<DeviceState::ValueType> ? std::sqrt(d2) : Math::ONE<DeviceState::ValueType>, dz, false);
		}

		return c;
	}

	DeviceState::CountType Gamepad::_getTrigger(SHORT t, GamepadKeyCode key, DeviceState::ValueType& data) const {
		auto dz = _getDeadZone(key);

		data = _translateTrigger(t);
		data = _translateDeadZone01(data, dz, data <= dz);

		return 1;
	}

	void Gamepad::_dispatchStick(SHORT oriX, SHORT oriY, SHORT curX, SHORT curY, GamepadKeyCode key) {
		DeviceState::ValueType value[] = { _translateStick<false>(curX) , _translateStick<true>(curY) };
		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;
		auto x = _translateStick<false>(oriX), y = _translateStick<true>(oriY);
		auto oriDz = x * x + y * y <= dz2;
		auto d2 = value[0] * value[0] + value[1] * value[1];
		if (d2 > Math::ONE<DeviceState::ValueType>) d2 = Math::ONE<DeviceState::ValueType>;
		auto curDz = d2 <= dz2;
		if (!oriDz || oriDz != curDz) {
			if (curDz) {
				value[0] = Math::NEGATIVE_ONE<DeviceState::ValueType>;
				value[1] = Math::ZERO<DeviceState::ValueType>;
			} else {
				value[0] = std::atan2(value[1], value[0]) + Math::PI_2<float32_t>;
				if (value[0] < Math::ZERO<DeviceState::ValueType>) value[0] += Math::PI2<float32_t>;
				value[1] = _translateDeadZone01(d2 < Math::ONE<DeviceState::ValueType> ? std::sqrt(d2) : Math::ONE<DeviceState::ValueType>, dz, false);
			}
			DeviceState k = { (DeviceState::CodeType)key, 2, value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void Gamepad::_dispatchTrigger(SHORT ori, SHORT cur, GamepadKeyCode key) {
		auto value = _translateTrigger(cur);
		auto dz = _getDeadZone(key);
		auto oriDz = _translateTrigger(ori) <= dz;
		auto curDz = value <= dz;
		if (!curDz || oriDz != curDz) {
			value = _translateDeadZone01(value, dz, curDz);
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void Gamepad::_dispatchButton(WORD ori, WORD cur, uint16_t flags, GamepadKeyCode key) {
		auto curDown = cur & flags;
		if ((ori & flags) != curDown) {
			auto value = curDown ? Math::ONE<DeviceState::ValueType> : Math::ZERO<DeviceState::ValueType>;
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher.dispatchEvent(this, curDown ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}
}