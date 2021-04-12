#include "Gamepad.h"
#include "Input.h"

#include <wbemidl.h>
#include <oleauto.h>
#include <dinput.h>

namespace aurora::modules::inputs::direct_input {
	const Gamepad::KeyMapping Gamepad::DIRECT{
			0, 1, 2, 5, 3, 4,
			{
			{ 0, GamepadKeyCode::X },
			{ 1, GamepadKeyCode::A },
			{ 2, GamepadKeyCode::B },
			{ 3, GamepadKeyCode::Y },
			{ 4, GamepadKeyCode::L_SHOULDER },
			{ 5, GamepadKeyCode::R_SHOULDER },
			{ 6, GamepadKeyCode::L_TRIGGER },
			{ 7, GamepadKeyCode::R_TRIGGER },
			{ 8, GamepadKeyCode::SELECT },
			{ 9, GamepadKeyCode::START },
			{ 10, GamepadKeyCode::L_THUMB },
			{ 11, GamepadKeyCode::R_THUMB }
			}
	};

	const Gamepad::KeyMapping Gamepad::XINPUT{
			0, 1, 3, 4, 2, 2,
			{
			{ 0, GamepadKeyCode::A },
			{ 1, GamepadKeyCode::B },
			{ 2, GamepadKeyCode::X },
			{ 3, GamepadKeyCode::Y },
			{ 4, GamepadKeyCode::L_SHOULDER },
			{ 5, GamepadKeyCode::R_SHOULDER },
			{ 6, GamepadKeyCode::SELECT },
			{ 7, GamepadKeyCode::START },
			{ 8, GamepadKeyCode::L_THUMB },
			{ 9, GamepadKeyCode::R_THUMB }
			}
	};

	const Gamepad::KeyMapping Gamepad::DS4{
			0, 1, 2, 5, 3, 4,
			{
			{ 0, GamepadKeyCode::X },
			{ 1, GamepadKeyCode::A },
			{ 2, GamepadKeyCode::B },
			{ 3, GamepadKeyCode::Y },
			{ 4, GamepadKeyCode::L_SHOULDER },
			{ 5, GamepadKeyCode::R_SHOULDER },
			{ 6, GamepadKeyCode::L_TRIGGER },
			{ 7, GamepadKeyCode::R_TRIGGER },
			{ 8, GamepadKeyCode::SELECT },
			{ 9, GamepadKeyCode::START },
			{ 10, GamepadKeyCode::L_THUMB },
			{ 11, GamepadKeyCode::R_THUMB },
			{ 13, GamepadKeyCode::TOUCH_PAD }
			}
	};

	Gamepad::Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info) : DeviceBase(input, dev, info),
		_keyMapping(nullptr) {
		_dev->SetDataFormat(&c_dfDIJoystick2);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

		if (_info.isXInput) {
			_keyMapping = &XINPUT;
		} else {
			auto data = (const uint16_t*)info.guid.getData();
			auto vender = data[0];
			auto product = data[1];
			switch (vender) {
			case 0x054C:
			{
				if (product == 0x05C4 || product == 0x09CC) _keyMapping = &DS4;

				break;
			}
			default:
				break;
			}
		}

		if (!_keyMapping) _keyMapping = &DIRECT;
		for (auto& itr : _keyMapping->BUTTONS) _enumToKeyMapping.try_emplace(itr.second, itr.first);

		memset(&_state, 0, sizeof(_state));
		memset(&_state.rgdwPOV, 0xFF, sizeof(_state.rgdwPOV));

		using AxisType = typename std::remove_cvref_t<decltype(_state.lX)>;

		auto axis = &_state.lX;
		axis[_keyMapping->LSTICK_X] = NUMBER_32767<AxisType>;
		axis[_keyMapping->LSTICK_Y] = NUMBER_32767<AxisType>;
		axis[_keyMapping->RSTICK_X] = NUMBER_32767<AxisType>;
		axis[_keyMapping->RSTICK_Y] = NUMBER_32767<AxisType>;
		if (_keyMapping->LTRIGGER == _keyMapping->RTRIGGER) axis[_keyMapping->LTRIGGER] = NUMBER_32767<AxisType>;;

		_setDeadZone(GamepadKeyCode::L_STICK, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadKeyCode::R_STICK, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadKeyCode::L_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadKeyCode::R_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				std::shared_lock lock(_mutex);

				switch ((GamepadKeyCode)code) {
				case GamepadKeyCode::L_STICK:
				{
					auto axis = &_state.lX;
					return _getStick(axis[_keyMapping->LSTICK_X], axis[_keyMapping->LSTICK_Y], (GamepadKeyCode)code, (DeviceStateValue*)values, count);
				}
				case GamepadKeyCode::R_STICK:
				{
					auto axis = &_state.lX;
					return _getStick(axis[_keyMapping->RSTICK_X], axis[_keyMapping->RSTICK_Y], (GamepadKeyCode)code, (DeviceStateValue*)values, count);
				}
				case GamepadKeyCode::L_TRIGGER:
				{
					auto axis = &_state.lX;
					return _keyMapping->LTRIGGER == _keyMapping->RTRIGGER ? _getTrigger(axis[_keyMapping->LTRIGGER], (GamepadKeyCode)code, 0, ((DeviceStateValue*)values)[0]) : _getTriggerSeparate(axis[_keyMapping->LTRIGGER], (GamepadKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadKeyCode::R_TRIGGER:
				{
					auto axis = &_state.lX;
					return _keyMapping->LTRIGGER == _keyMapping->RTRIGGER ? _getTrigger(axis[_keyMapping->RTRIGGER], (GamepadKeyCode)code, 1, ((DeviceStateValue*)values)[0]) : _getTriggerSeparate(axis[_keyMapping->RTRIGGER], (GamepadKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadKeyCode::DPAD:
					((DeviceStateValue*)values)[0] = _translateDpad(_state.rgdwPOV[0]);
					return 1;
				default:
				{
					if (auto itr = _enumToKeyMapping.find((GamepadKeyCode)code); itr != _enumToKeyMapping.end()) {
						((DeviceStateValue*)values)[0] = _translateButton(_state.rgbButtons[itr->second]);

						return 1;
					}

					if (code >= (uint8_t)GamepadKeyCode::UNDEFINED) {
						((DeviceStateValue*)values)[0] = _translateButton(_state.rgbButtons[code - (DeviceState::CodeType)GamepadKeyCode::UNDEFINED]);

						return 1;
					}

					break;
				}
				}
			}

			return 0;
		}
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

	void Gamepad::poll(bool dispatchEvent) {
		using namespace aurora::enum_operators;

		if (auto hr = _dev->Poll(); hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		DIJOYSTATE2 state;
		if (FAILED(_dev->GetDeviceState(sizeof(state), &state))) return;
		if (_checkInvalidData(state)) return;

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(&_state, &state, sizeof(_state));

			return;
		}

		auto curAxis = &state.lX;

		LONG oriLStickX, oriLStickY, oriRStickX, oriRStickY;
		auto ls = false, rs = false;

		auto lt = false, rt = false;
		LONG oriLT, oriRT;

		uint8_t changedBtns[sizeof(state.rgbButtons)];
		uint8_t changedBtnsLen = 0;

		uint8_t changedPov[4];
		uint8_t changedPovLen = 0;

		{
			std::scoped_lock lock(_mutex);

			auto oriAxis = &_state.lX;

			if (oriAxis[_keyMapping->LSTICK_X] != curAxis[_keyMapping->LSTICK_X] || oriAxis[_keyMapping->LSTICK_Y] != curAxis[_keyMapping->LSTICK_Y]) {
				oriLStickX = oriAxis[_keyMapping->LSTICK_X];
				oriLStickY = oriAxis[_keyMapping->LSTICK_Y];
				oriAxis[_keyMapping->LSTICK_X] = curAxis[_keyMapping->LSTICK_X];
				oriAxis[_keyMapping->LSTICK_Y] = curAxis[_keyMapping->LSTICK_Y];
				ls = true;
			}
			if (oriAxis[_keyMapping->RSTICK_X] != curAxis[_keyMapping->RSTICK_X] || oriAxis[_keyMapping->RSTICK_Y] != curAxis[_keyMapping->RSTICK_Y]) {
				oriRStickX = oriAxis[_keyMapping->RSTICK_X];
				oriRStickY = oriAxis[_keyMapping->RSTICK_Y];
				oriAxis[_keyMapping->RSTICK_X] = curAxis[_keyMapping->RSTICK_X];
				oriAxis[_keyMapping->RSTICK_Y] = curAxis[_keyMapping->RSTICK_Y];
				rs = true;
			}

			if (oriAxis[_keyMapping->LTRIGGER] != curAxis[_keyMapping->LTRIGGER]) {
				oriLT = oriAxis[_keyMapping->LTRIGGER];
				oriAxis[_keyMapping->LTRIGGER] = curAxis[_keyMapping->LTRIGGER];
				lt = true;
			}
			if (_keyMapping->LTRIGGER != _keyMapping->RTRIGGER && oriAxis[_keyMapping->RTRIGGER] != curAxis[_keyMapping->RTRIGGER]) {
				oriRT = oriAxis[_keyMapping->RTRIGGER];
				oriAxis[_keyMapping->RTRIGGER] = curAxis[_keyMapping->RTRIGGER];
				rt = true;
			}

			for (uint8_t i = 0; i < sizeof(state.rgbButtons); ++i) {
				if (_state.rgbButtons[i] != state.rgbButtons[i]) {
					_state.rgbButtons[i] = state.rgbButtons[i];
					changedBtns[changedBtnsLen++] = i;
				}
			}

			for (uint8_t i = 0; i < sizeof(state.rgdwPOV); ++i) {
				if (_state.rgdwPOV[i] != state.rgdwPOV[i]) {
					_state.rgdwPOV[i] = state.rgdwPOV[i];
					changedPov[changedPovLen++] = i;
				}
			}
		}

		if (ls) _dispatchStick(oriLStickX, oriLStickY, curAxis[_keyMapping->LSTICK_X], curAxis[_keyMapping->LSTICK_Y], GamepadKeyCode::L_STICK);
		if (rs) _dispatchStick(oriRStickX, oriRStickY, curAxis[_keyMapping->RSTICK_X], curAxis[_keyMapping->RSTICK_Y], GamepadKeyCode::R_STICK);

		if (_keyMapping->LTRIGGER == _keyMapping->RTRIGGER) {
			if (lt) _dispatchTrigger(oriLT, curAxis[_keyMapping->LTRIGGER], GamepadKeyCode::L_TRIGGER, GamepadKeyCode::R_TRIGGER);
		} else {
			if (lt) _dispatchTriggerSeparate(oriLT, curAxis[_keyMapping->LTRIGGER], GamepadKeyCode::L_TRIGGER);
			if (rt) _dispatchTriggerSeparate(oriRT, curAxis[_keyMapping->RTRIGGER], GamepadKeyCode::R_TRIGGER);
		}

		if (changedPovLen) {
			for (decltype(changedPovLen) i = 0; i < changedPovLen; ++i) {
				DeviceState::CodeType key = changedPov[i];
				if (key == 0) {
					auto value = _translateDpad(state.rgdwPOV[key]);
					DeviceState k = { (DeviceState::CodeType)GamepadKeyCode::DPAD, 1, &value };
					_eventDispatcher->dispatchEvent(this, value >= Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
				}
			}
		}

		if (changedBtnsLen) {
			for (decltype(changedBtnsLen) i = 0; i < changedBtnsLen; ++i) {
				DeviceState::CodeType key = changedBtns[i];
				auto value = _translateButton(state.rgbButtons[key]);

				auto itr = _keyMapping->BUTTONS.find(key);
				key = (DeviceState::CodeType)(itr == _keyMapping->BUTTONS.end() ? key + GamepadKeyCode::UNDEFINED : itr->second);

				DeviceState k = { key, 1, &value };
				_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
			}
		}
	}

	void Gamepad::_setDeadZone(GamepadKeyCode keyCode, DeviceStateValue deadZone) {
		if (deadZone < Math::ZERO<DeviceStateValue>) deadZone = -deadZone;

		std::scoped_lock lock(_deadZoneMutex);

		_deadZone.insert_or_assign(keyCode, deadZone);
	}

	bool Gamepad::_checkInvalidData(const DIJOYSTATE2& state) {
		auto axis = &state.lX;
		for (uint32_t i = 0; i < 6; ++i) {
			if (axis[i] != 32767) return false;
		}

		constexpr auto numButtons = sizeof(state.rgbButtons) / sizeof(state.rgbButtons[0]);
		for (std::remove_cvref_t<decltype(numButtons)> i = 0; i < numButtons; ++i) {
			if (state.rgbButtons[i] != 0) return false;
		}

		return true;
	}

	DeviceStateValue Gamepad::_translateStick(LONG value) {
		auto v = DeviceStateValue(value - NUMBER_32767<decltype(value)>);
		if (v < Math::ZERO<DeviceStateValue>) {
			v *= Math::RECIPROCAL<NUMBER_32767<DeviceStateValue>>;
		} else if (v > Math::ZERO<DeviceStateValue>) {
			v *= Math::RECIPROCAL<NUMBER_32768<DeviceStateValue>>;
		}
		return v;
	}

	void Gamepad::_translateTrigger(LONG value, DeviceStateValue& l, DeviceStateValue& r) {
		using ValType = typename std::remove_cvref_t<decltype(value)>;

		if (value < NUMBER_32767<ValType>) {
			l = Math::ZERO<DeviceStateValue>;
			r = DeviceStateValue(NUMBER_32767<ValType> - value) * Math::RECIPROCAL<NUMBER_32767<DeviceStateValue>>;
		} else if (value > NUMBER_32767<ValType>) {
			l = DeviceStateValue(value - NUMBER_32767<ValType>) * Math::RECIPROCAL<NUMBER_32768<DeviceStateValue>>;
			r = Math::ZERO<DeviceStateValue>;
		} else {
			l = Math::ZERO<DeviceStateValue>;
			r = Math::ZERO<DeviceStateValue>;
		}
	}

	DeviceState::CountType Gamepad::_getStick(LONG x, LONG y, GamepadKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
		DeviceState::CountType c = 1;

		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;

		auto dx = _translateStick(x);
		auto dy = _translateStick(y);

		auto d2 = dx * dx + dy * dy;
		if (d2 > Math::ONE<DeviceStateValue>) d2 = Math::ONE<DeviceStateValue>;

		if (d2 <= dz2) {
			data[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
			if (count > 1) data[c++] = Math::ZERO<DeviceStateValue>;
		} else {
			data[0] = std::atan2(dy, dx) + Math::PI_2<float32_t>;
			if (data[0] < 0.f) data[0] += Math::PI2<float32_t>;
			if (count > 1) data[c++] = _translateDeadZone01(d2 < Math::ONE<DeviceStateValue> ? std::sqrt(d2) : Math::ONE<DeviceStateValue>, dz, false);
		}

		return c;
	}

	DeviceState::CountType Gamepad::_getTrigger(LONG t, GamepadKeyCode key, uint8_t index, DeviceStateValue& data) const {
		auto dz = _getDeadZone(key);

		DeviceStateValue values[2];
		_translateTrigger(t, values[0], values[1]);
		data = values[index];
		data = _translateDeadZone01(data, dz, data <= dz);

		return 1;
	}

	DeviceState::CountType Gamepad::_getTriggerSeparate(LONG t, GamepadKeyCode key, DeviceStateValue& data) const {
		auto dz = _getDeadZone(key);

		data = _translateTriggerSeparate(t);
		data = _translateDeadZone01(data, dz, data <= dz);

		return 1;
	}

	void Gamepad::_dispatchStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadKeyCode key) {
		DeviceStateValue value[] = { _translateStick(curX) , _translateStick(curY) };
		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;
		auto x = _translateStick(oriX), y = _translateStick(oriY);
		auto oriDz = x * x + y * y <= dz2;
		auto d2 = value[0] * value[0] + value[1] * value[1];
		if (d2 > Math::ONE<DeviceStateValue>) d2 = Math::ONE<DeviceStateValue>;
		auto curDz = d2 <= dz2;
		if (!oriDz || oriDz != curDz) {
			if (curDz) {
				value[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
				value[1] = Math::ZERO<DeviceStateValue>;
			} else {
				value[0] = std::atan2(value[1], value[0]) + Math::PI_2<DeviceStateValue>;
				if (value[0] < Math::ZERO<DeviceStateValue>) value[0] += Math::PI2<DeviceStateValue>;
				value[1] = _translateDeadZone01(d2 < Math::ONE<DeviceStateValue> ? std::sqrt(d2) : Math::ONE<DeviceStateValue>, dz, false);
			}

			DeviceState k = { (DeviceState::CodeType)key, 2, value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void Gamepad::_dispatchTrigger(LONG ori, LONG cur, GamepadKeyCode lkey, GamepadKeyCode rkey) {
		DeviceStateValue oriValues[2], curValues[2];
		_translateTrigger(ori, oriValues[0], oriValues[1]);
		_translateTrigger(cur, curValues[0], curValues[1]);
		auto ldz = _getDeadZone(lkey);
		auto rdz = _getDeadZone(rkey);
		auto oriLDz = oriValues[0] <= ldz;
		auto curLDz = curValues[0] <= ldz;
		auto oriRDz = oriValues[1] <= rdz;
		auto curRDz = curValues[1] <= rdz;
		if (!curLDz || oriLDz != curLDz) {
			curValues[0] = _translateDeadZone01(curValues[0], ldz, curLDz);
			DeviceState k = { (DeviceState::CodeType)lkey, 1, &curValues[0] };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
		if (!curRDz || oriRDz != curRDz) {
			curValues[1] = _translateDeadZone01(curValues[1], rdz, curRDz);
			DeviceState k = { (DeviceState::CodeType)rkey, 1, &curValues[1] };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void Gamepad::_dispatchTriggerSeparate(LONG ori, LONG cur, GamepadKeyCode key) {
		auto value = _translateTriggerSeparate(cur);
		auto dz = _getDeadZone(key);
		auto oriDz = _translateTriggerSeparate(ori) <= dz;
		auto curDz = value <= dz;
		if (!curDz || oriDz != curDz) {
			value = _translateDeadZone01(value, dz, curDz);
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}
}