#include "Gamepad.h"
#include "Input.h"

#include <wbemidl.h>
#include <oleauto.h>
#include <dinput.h>

namespace aurora::modules::inputs::direct_input {
	const Gamepad::KeyMapping Gamepad::DIRECT = Gamepad::_createKeyMapping<
		GamepadVirtualKeyCode::X,
		GamepadVirtualKeyCode::A,
		GamepadVirtualKeyCode::B,
		GamepadVirtualKeyCode::Y,
		GamepadVirtualKeyCode::L_SHOULDER,
		GamepadVirtualKeyCode::R_SHOULDER,
		GamepadVirtualKeyCode::UNKNOWN,
		GamepadVirtualKeyCode::UNKNOWN,
		GamepadVirtualKeyCode::SELECT,
		GamepadVirtualKeyCode::START,
		GamepadVirtualKeyCode::L_THUMB,
		GamepadVirtualKeyCode::R_THUMB
		>(GamepadKeyCode::X, GamepadKeyCode::Y, GamepadKeyCode::Z, GamepadKeyCode::RZ, GamepadKeyCode::RX, GamepadKeyCode::RY);

	const Gamepad::KeyMapping Gamepad::XINPUT = Gamepad::_createKeyMapping<
		GamepadVirtualKeyCode::A,
		GamepadVirtualKeyCode::B,
		GamepadVirtualKeyCode::X,
		GamepadVirtualKeyCode::Y,
		GamepadVirtualKeyCode::L_SHOULDER,
		GamepadVirtualKeyCode::R_SHOULDER,
		GamepadVirtualKeyCode::SELECT,
		GamepadVirtualKeyCode::START,
		GamepadVirtualKeyCode::L_THUMB,
		GamepadVirtualKeyCode::R_THUMB
	>(GamepadKeyCode::X, GamepadKeyCode::Y, GamepadKeyCode::RX, GamepadKeyCode::RY, GamepadKeyCode::Z, GamepadKeyCode::Z);

	const Gamepad::KeyMapping Gamepad::DS4 = Gamepad::_createKeyMapping<
		GamepadVirtualKeyCode::X,
		GamepadVirtualKeyCode::A,
		GamepadVirtualKeyCode::B,
		GamepadVirtualKeyCode::Y,
		GamepadVirtualKeyCode::L_SHOULDER,
		GamepadVirtualKeyCode::R_SHOULDER,
		GamepadVirtualKeyCode::UNKNOWN,
		GamepadVirtualKeyCode::UNKNOWN,
		GamepadVirtualKeyCode::SELECT,
		GamepadVirtualKeyCode::START,
		GamepadVirtualKeyCode::L_THUMB,
		GamepadVirtualKeyCode::R_THUMB,
		GamepadVirtualKeyCode::TOUCH_PAD
	>(GamepadKeyCode::X, GamepadKeyCode::Y, GamepadKeyCode::Z, GamepadKeyCode::RZ, GamepadKeyCode::RX, GamepadKeyCode::RY);

	Gamepad::Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIJoystick);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

		memset(&_state.rgdwPOV, 0xFF, sizeof(_state.rgdwPOV));

		_setKeyMapping(nullptr);

		_setDeadZone(GamepadVirtualKeyCode::L_STICK, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadVirtualKeyCode::R_STICK, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadVirtualKeyCode::L_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadVirtualKeyCode::R_TRIGGER, Math::TWENTIETH<DeviceStateValue>);
	}

	DeviceState::CountType Gamepad::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		using namespace aurora::enum_operators;

		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				std::shared_lock lock(_mutex);

				switch ((GamepadVirtualKeyCode)code) {
				case GamepadVirtualKeyCode::L_STICK:
				{
					auto axis = &_state.lX;
					return _getStick(axis[_keyMapping.lStick[0]], axis[_keyMapping.lStick[1]], (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				}
				case GamepadVirtualKeyCode::R_STICK:
				{
					auto axis = &_state.lX;
					return _getStick(axis[_keyMapping.rStick[0]], axis[_keyMapping.rStick[1]], (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				}
				case GamepadVirtualKeyCode::L_TRIGGER:
				{
					auto axis = &_state.lX;
					return _keyMapping.trigger[0] == _keyMapping.trigger[1] ? _getTrigger(axis[_keyMapping.trigger[0]], (GamepadVirtualKeyCode)code, 0, ((DeviceStateValue*)values)[0]) : _getTriggerSeparate(axis[_keyMapping.trigger[0]], (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadVirtualKeyCode::R_TRIGGER:
				{
					auto axis = &_state.lX;
					return _keyMapping.trigger[0] == _keyMapping.trigger[1] ? _getTrigger(axis[_keyMapping.trigger[1]], (GamepadVirtualKeyCode)code, 1, ((DeviceStateValue*)values)[0]) : _getTriggerSeparate(axis[_keyMapping.trigger[1]], (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadVirtualKeyCode::DPAD:
					((DeviceStateValue*)values)[0] = _translateDpad(_state.rgdwPOV[0]);
					return 1;
				default:
				{
					if (auto itr = _enumToKeyMapping.find((GamepadVirtualKeyCode)code); itr != _enumToKeyMapping.end()) {
						((DeviceStateValue*)values)[0] = _translateButton(_state.rgbButtons[itr->second]);

						return 1;
					}

					return 0;
				}
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
		case DeviceStateType::KEY_MAPPING:
		{
			if (values && count) {
				std::scoped_lock lock(_mutex);

				_setKeyMapping((const SerializableObject*)values);
				return 1;
			}

			return 0;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				_setDeadZone((GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
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

		DIJOYSTATE state;
		if (FAILED(_dev->GetDeviceState(sizeof(state), &state))) return;
		//if (_checkInvalidData(state)) return;

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(&_state, &state, sizeof(_state));

			return;
		}

		auto curAxis = &state.lX;

		KeyMapping keyMapping;
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

			keyMapping = _keyMapping;
			auto oriAxis = &_state.lX;

			if (oriAxis[keyMapping.lStick[0]] != curAxis[keyMapping.lStick[0]] || oriAxis[keyMapping.lStick[1]] != curAxis[keyMapping.lStick[1]]) {
				oriLStickX = oriAxis[keyMapping.lStick[0]];
				oriLStickY = oriAxis[keyMapping.lStick[1]];
				oriAxis[keyMapping.lStick[0]] = curAxis[keyMapping.lStick[0]];
				oriAxis[keyMapping.lStick[1]] = curAxis[keyMapping.lStick[1]];
				ls = true;
			}
			if (oriAxis[keyMapping.rStick[0]] != curAxis[keyMapping.rStick[0]] || oriAxis[keyMapping.rStick[1]] != curAxis[keyMapping.rStick[1]]) {
				oriRStickX = oriAxis[keyMapping.rStick[0]];
				oriRStickY = oriAxis[keyMapping.rStick[1]];
				oriAxis[keyMapping.rStick[0]] = curAxis[keyMapping.rStick[0]];
				oriAxis[keyMapping.rStick[1]] = curAxis[keyMapping.rStick[1]];
				rs = true;
			}

			if (oriAxis[keyMapping.trigger[0]] != curAxis[keyMapping.trigger[0]]) {
				oriLT = oriAxis[keyMapping.trigger[0]];
				oriAxis[keyMapping.trigger[0]] = curAxis[keyMapping.trigger[0]];
				lt = true;
			}
			if (keyMapping.trigger[0] != keyMapping.trigger[1] && oriAxis[keyMapping.trigger[1]] != curAxis[keyMapping.trigger[1]]) {
				oriRT = oriAxis[keyMapping.trigger[1]];
				oriAxis[keyMapping.trigger[1]] = curAxis[keyMapping.trigger[1]];
				rt = true;
			}

			for (uint8_t i = 0; i < sizeof(state.rgbButtons); ++i) {
				if (_state.rgbButtons[i] != state.rgbButtons[i]) {
					_state.rgbButtons[i] = state.rgbButtons[i];
					changedBtns[changedBtnsLen++] = i;
				}
			}

			for (uint8_t i = 0; i < 4; ++i) {
				if (_state.rgdwPOV[i] != state.rgdwPOV[i]) {
					_state.rgdwPOV[i] = state.rgdwPOV[i];
					changedPov[changedPovLen++] = i;
				}
			}
		}

		if (ls) _dispatchStick(oriLStickX, oriLStickY, curAxis[keyMapping.lStick[0]], curAxis[keyMapping.lStick[1]], GamepadVirtualKeyCode::L_STICK);
		if (rs) _dispatchStick(oriRStickX, oriRStickY, curAxis[keyMapping.rStick[0]], curAxis[keyMapping.rStick[1]], GamepadVirtualKeyCode::R_STICK);

		if (keyMapping.trigger[0] == keyMapping.trigger[1]) {
			if (lt) _dispatchTrigger(oriLT, curAxis[keyMapping.trigger[0]], GamepadVirtualKeyCode::L_TRIGGER, GamepadVirtualKeyCode::R_TRIGGER);
		} else {
			if (lt) _dispatchTriggerSeparate(oriLT, curAxis[keyMapping.trigger[0]], GamepadVirtualKeyCode::L_TRIGGER);
			if (rt) _dispatchTriggerSeparate(oriRT, curAxis[keyMapping.trigger[1]], GamepadVirtualKeyCode::R_TRIGGER);
		}

		for (decltype(changedPovLen) i = 0; i < changedPovLen; ++i) {
			DeviceState::CodeType key = changedPov[i];
			if (key == 0) {
				auto value = _translateDpad(state.rgdwPOV[key]);
				DeviceState k = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &value };
				_eventDispatcher->dispatchEvent(this, value >= Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
			}
		}

		for (decltype(changedBtnsLen) i = 0; i < changedBtnsLen; ++i) {
			DeviceState::CodeType key = changedBtns[i];
			auto value = _translateButton(state.rgbButtons[key]);

			auto vkey = (DeviceState::CodeType)_keyMapping.buttons[key];
			if (vkey == GamepadVirtualKeyCode::UNKNOWN) continue;

			DeviceState k = { vkey, 1, &value };
			_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}

	void Gamepad::_setDeadZone(GamepadVirtualKeyCode keyCode, DeviceStateValue deadZone) {
		if (deadZone < Math::ZERO<DeviceStateValue>) deadZone = -deadZone;

		std::scoped_lock lock(_deadZoneMutex);

		_deadZone.insert_or_assign(keyCode, deadZone);
	}

	bool Gamepad::_checkInvalidData(const DIJOYSTATE& state) {
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

	DeviceState::CountType Gamepad::_getStick(LONG x, LONG y, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
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

	DeviceState::CountType Gamepad::_getTrigger(LONG t, GamepadVirtualKeyCode key, uint8_t index, DeviceStateValue& data) const {
		auto dz = _getDeadZone(key);

		DeviceStateValue values[2];
		_translateTrigger(t, values[0], values[1]);
		data = values[index];
		data = _translateDeadZone01(data, dz, data <= dz);

		return 1;
	}

	DeviceState::CountType Gamepad::_getTriggerSeparate(LONG t, GamepadVirtualKeyCode key, DeviceStateValue& data) const {
		auto dz = _getDeadZone(key);

		data = _translateTriggerSeparate(t);
		data = _translateDeadZone01(data, dz, data <= dz);

		return 1;
	}

	void Gamepad::_dispatchStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadVirtualKeyCode key) {
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

	void Gamepad::_dispatchTrigger(LONG ori, LONG cur, GamepadVirtualKeyCode lkey, GamepadVirtualKeyCode rkey) {
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

	void Gamepad::_dispatchTriggerSeparate(LONG ori, LONG cur, GamepadVirtualKeyCode key) {
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

	void Gamepad::_setKeyMapping(const SerializableObject* mapping) {
		using namespace aurora::enum_operators;

		if (mapping) {
			memset(_keyMapping.axis, 0, sizeof(_keyMapping.axis));
			memset(_keyMapping.buttons, (uint8_t)GamepadVirtualKeyCode::UNDEFINED_BUTTON, sizeof(_keyMapping.buttons));

			mapping->forEach([this](const SerializableObject& key, const SerializableObject& val) {
				auto vc = key.toEnum<GamepadVirtualKeyCode>();
				switch (vc) {
				case GamepadVirtualKeyCode::L_STICK:
				{
					if (val.getType() == SerializableObject::Type::ARRAY) {
						for (auto i = 0; i < 2; ++i) {
							if (auto v = val.tryAt(i).toEnum<GamepadKeyCode>(); v >= GamepadKeyCode::X && v <= GamepadKeyCode::RZ) _keyMapping.lStick[i] = (uint8_t)(v);
						}
					} else {
						if (auto v = val.toEnum<GamepadKeyCode>();  v >= GamepadKeyCode::X && v <= GamepadKeyCode::RZ) _keyMapping.lStick[0] = (uint8_t)(v);
					}

					break;
				}
				case GamepadVirtualKeyCode::R_STICK:
				{
					if (val.getType() == SerializableObject::Type::ARRAY) {
						for (auto i = 0; i < 2; ++i) {
							if (auto v = val.tryAt(i).toEnum<GamepadKeyCode>(); v >= GamepadKeyCode::X && v <= GamepadKeyCode::RZ) _keyMapping.rStick[i] = (uint8_t)(v);
						}
					} else {
						if (auto v = val.toEnum<GamepadKeyCode>();  v >= GamepadKeyCode::X && v <= GamepadKeyCode::RZ) _keyMapping.rStick[0] = (uint8_t)(v);
					}

					break;
				}
				case GamepadVirtualKeyCode::L_TRIGGER:
				{
					if (auto v = val.toEnum<GamepadKeyCode>();  v >= GamepadKeyCode::X && v <= GamepadKeyCode::RZ) _keyMapping.trigger[0] = (uint8_t)(v);

					break;
				}
				case GamepadVirtualKeyCode::R_TRIGGER:
				{
					if (auto v = val.toEnum<GamepadKeyCode>();  v >= GamepadKeyCode::X && v <= GamepadKeyCode::RZ) _keyMapping.trigger[1] = (uint8_t)(v);

					break;
				}
				default:
				{
					if (vc >= GamepadVirtualKeyCode::BUTTON_START) {
						if (auto v = val.toEnum<GamepadKeyCode>();  v >= GamepadKeyCode::BUTTON_1) _keyMapping.buttons[(std::underlying_type_t<GamepadKeyCode>)(v - GamepadKeyCode::BUTTON_1)] = vc;
					}

					break;
				}
				}
			});
		} else {
			if (_info.isXInput) {
				_keyMapping = XINPUT;
			} else {
				auto data = (const uint16_t*)_info.guid.getData();
				auto vender = data[0];
				auto product = data[1];
				switch (vender) {
				case 0x054C:
				{
					if (product == 0x05C4 || product == 0x09CC) _keyMapping = DS4;

					break;
				}
				default:
					_keyMapping = DIRECT;
					break;
				}
			}
		}

		if (_keyMapping.lStick[0] == 0) _keyMapping.lStick[0] = (uint8_t)GamepadKeyCode::X;
		if (_keyMapping.lStick[1] == 0) _keyMapping.lStick[1] = (uint8_t)GamepadKeyCode::Y;
		if (_keyMapping.rStick[0] == 0) _keyMapping.rStick[0] = (uint8_t)GamepadKeyCode::RX;
		if (_keyMapping.rStick[1] == 0) _keyMapping.lStick[1] = (uint8_t)GamepadKeyCode::RY;
		if (_keyMapping.trigger[0] == 0) _keyMapping.trigger[0] = (uint8_t)GamepadKeyCode::Z;
		if (_keyMapping.trigger[1] == 0) _keyMapping.trigger[1] = _keyMapping.trigger[0];

		for (size_t i = 0; i < 6; ++i) _keyMapping.axis[i] -= (uint8_t)GamepadKeyCode::AXIS_START;

		uint_t<MAX_BUTTONS> btns = 0;
		for (size_t i = 0; i < MAX_BUTTONS; ++i) {
			if (_keyMapping.buttons[i] >= GamepadVirtualKeyCode::UNDEFINED_BUTTON_1) btns |= 1 << (uint32_t)(_keyMapping.buttons[i] - GamepadVirtualKeyCode::UNDEFINED_BUTTON_1);
		}
		for (size_t i = 0; i < MAX_BUTTONS; ++i) {
			if (_keyMapping.buttons[i] == GamepadVirtualKeyCode::UNDEFINED_BUTTON) {
				auto idx = 0;
				for (size_t j = 0; j < MAX_BUTTONS; ++j) {
					if ((btns & (1 << j)) == 0) {
						btns |= 1 << j;
						idx = j;
						break;
					}
				}

				_keyMapping.buttons[i] = GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 + idx;
			}
		}

		_enumToKeyMapping.clear();
		for (size_t i = 0; i < MAX_BUTTONS; ++i) {
			if (_keyMapping.buttons[i] != GamepadVirtualKeyCode::UNKNOWN) {
				_enumToKeyMapping.try_emplace(_keyMapping.buttons[i], i);
			}
		}

		memset(&_state.rgbButtons, 0, sizeof(_state.rgbButtons));

		using AxisType = typename std::remove_cvref_t<decltype(_state.lX)>;

		auto axis = &_state.lX;
		for (size_t i = 0; i < 4; ++i) {
			axis[_keyMapping.axis[i]] = NUMBER_32767<AxisType>;
		}
		if (_keyMapping.trigger[0] == _keyMapping.trigger[1]) axis[_keyMapping.trigger[0]] = NUMBER_32767<AxisType>;
	}
}