#include "Gamepad.h"
#include "Input.h"
#include <bitset>

#include <wbemidl.h>
#include <oleauto.h>
#include <dinput.h>

namespace aurora::modules::inputs::direct_input {
	Gamepad::Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info) : DeviceBase(input, dev, info) {
		_dev->SetDataFormat(&c_dfDIJoystick);
		_dev->SetCooperativeLevel(_input->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

		_setKeyMapping(nullptr);

		_readState(_state);
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
					return _getStick(GamepadVirtualKeyCode::L_STICK_X, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::R_STICK:
					return _getStick(GamepadVirtualKeyCode::R_STICK_X, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::L_TRIGGER:
				{
					GamepadKeyCode mappingVals[2];
					_keyMapping.get(GamepadVirtualKeyCode::L_TRIGGER, 2, mappingVals);
					return mappingVals[0] == mappingVals[1] ?
						_getCombinedTrigger(mappingVals[0], (GamepadVirtualKeyCode)code, 0, ((DeviceStateValue*)values)[0]) :
						_getAxis(mappingVals[0], (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadVirtualKeyCode::R_TRIGGER:
				{
					GamepadKeyCode mappingVals[2];
					_keyMapping.get(GamepadVirtualKeyCode::L_TRIGGER, 2, mappingVals);
					return mappingVals[0] == mappingVals[1] ? 
						_getCombinedTrigger(mappingVals[0], (GamepadVirtualKeyCode)code, 1, ((DeviceStateValue*)values)[0]) :
						_getAxis(mappingVals[1], (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
				}
				case GamepadVirtualKeyCode::DPAD:
					((DeviceStateValue*)values)[0] = _translateDpad(_state.rgdwPOV[0]);
					return 1;
				default:
				{
					if (code >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && code <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
						_getAxis(_keyMapping.get((GamepadVirtualKeyCode)code), (GamepadVirtualKeyCode)code, ((DeviceStateValue*)values)[0]);
						return 1;
					} else if (code >= GamepadVirtualKeyCode::BUTTON_START && code <= GamepadVirtualKeyCode::BUTTON_END) {
						((DeviceStateValue*)values)[0] = _translateButton(_readButtonVal(_state.rgbButtons, _keyMapping.get((GamepadVirtualKeyCode)code)));
						return 1;
					}

					return 0;
				}
				}
			}

			return 0;
		}
		case DeviceStateType::KEY_MAPPING:
		{
			if (values && count) {
				std::shared_lock lock(_mutex);

				*((GamepadKeyMapping*)values) = _keyMapping;

				return 1;
			}

			return 0;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				DeviceState::CountType c = 1;

				auto dz = _getDeadZone((GamepadVirtualKeyCode)code);
				((DeviceStateValue*)values)[0] = dz[0];
				if (count > 1) ((DeviceStateValue*)values)[c++] = dz[1];

				return c;
			}

			return 0;
		}
		default:
			return 0;
		}
	}

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::KEY_MAPPING:
		{
			if (!count) values = nullptr;

			{
				std::scoped_lock lock(_mutex);

				_setKeyMapping((const GamepadKeyMapping*)values);
			}

			return 1;
		}
		case DeviceStateType::DEAD_ZONE:
		{
			if (!count) values = nullptr;

			if (values) {
				DeviceState::CountType c = 1;

				Vec2<DeviceStateValue> dz;
				dz[0] = ((DeviceStateValue*)values)[0];
				if (count > 1) dz[c++] = ((DeviceStateValue*)values)[1];

				_setDeadZone((GamepadVirtualKeyCode)code, &dz);

				return c;
			} else {
				_setDeadZone((GamepadVirtualKeyCode)code, nullptr);
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

		DIJOYSTATE newState;
		if (!_readState(newState)) return;
		//if (_checkInvalidData(state)) return;

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(&_state, &newState, sizeof(_state));

			return;
		}

		DIJOYSTATE oldState;
		GamepadKeyMapping keyMapping(NO_INIT);
		{
			std::scoped_lock lock(_mutex);

			keyMapping = _keyMapping;
			memcpy(&oldState, &_state, sizeof(_state));
			memcpy(&_state, &newState, sizeof(_state));
		}

		auto oldAxes = &oldState.lX;
		auto oldBtns = oldState.rgbButtons;
		auto newAxes = &newState.lX;
		auto newBtns = newState.rgbButtons;

		GamepadKeyCode mappingVals[4];
		keyMapping.get(GamepadVirtualKeyCode::L_STICK_X, 4, mappingVals);
		for (size_t i = 0; i < 2; ++i) {
			auto idx = i << 1;
			_dispatchStick(
				_readAxisVal(oldAxes, mappingVals[idx], (std::numeric_limits<int16_t>::max)()),
				_readAxisVal(oldAxes, mappingVals[idx + 1], (std::numeric_limits<int16_t>::max)()),
				_readAxisVal(newAxes, mappingVals[idx], (std::numeric_limits<int16_t>::max)()),
				_readAxisVal(newAxes, mappingVals[idx + 1], (std::numeric_limits<int16_t>::max)()),
				GamepadVirtualKeyCode::L_STICK + i);
		}

		keyMapping.get(GamepadVirtualKeyCode::L_TRIGGER, 2, mappingVals);
		if (mappingVals[0] == mappingVals[1]) {
			_dispatchCombinedTrigger(_readAxisVal(oldAxes, mappingVals[0], (std::numeric_limits<int16_t>::max)()), _readAxisVal(newAxes, mappingVals[1], (std::numeric_limits<int16_t>::max)()));
		} else {
			for (size_t i = 0; i < 2; ++i) _dispatchAxis(_readAxisVal(oldAxes, mappingVals[i], 0), _readAxisVal(newAxes, mappingVals[i], 0), GamepadVirtualKeyCode::L_TRIGGER + i);
		}

		keyMapping.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCode k) {
			if (vk >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1 && vk <= GamepadVirtualKeyCode::AXIS_END) {
				_dispatchAxis(_readAxisVal(oldAxes, k, 0), _readAxisVal(newAxes, k, 0), vk);
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				if (auto newVal = _readButtonVal(newBtns, k); newVal != _readButtonVal(oldBtns, k)) {
					auto value = _translateButton(newVal);
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &value };
					_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
				}
			}
		});

		for (uint8_t i = 0; i < 4; ++i) {
			if (oldState.rgdwPOV[i] != newState.rgdwPOV[i]) {
				auto value = _translateDpad(newState.rgdwPOV[i]);
				DeviceState ds = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &value };
				_eventDispatcher->dispatchEvent(this, value >= Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
			}
		}
	}

	bool Gamepad::_readState(DIJOYSTATE& state) {
		if (auto hr = _dev->Poll(); hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return false;
			if (FAILED(_dev->Poll())) return false;
		}

		if (FAILED(_dev->GetDeviceState(sizeof(state), &state))) return false;

		return true;
	}

	void Gamepad::_setKeyMapping(const GamepadKeyMapping* mapping) {
		using namespace aurora::enum_operators;

		if (mapping) {
			_keyMapping = *mapping;
		} else {
			_keyMapping.clear();

			_keyMapping.set(GamepadVirtualKeyCode::L_STICK_X, GamepadKeyCode::AXIS_1);
			_keyMapping.set(GamepadVirtualKeyCode::L_STICK_Y, GamepadKeyCode::AXIS_1 + 1);
			_keyMapping.set(GamepadVirtualKeyCode::R_STICK_X, GamepadKeyCode::AXIS_1 + 3);
			_keyMapping.set(GamepadVirtualKeyCode::R_STICK_Y, GamepadKeyCode::AXIS_1 + 4);
			_keyMapping.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 2);
			_keyMapping.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 2);

			_keyMapping.set(GamepadVirtualKeyCode::A, GamepadKeyCode::BUTTON_1);
			_keyMapping.set(GamepadVirtualKeyCode::B, GamepadKeyCode::BUTTON_1 + 1);
			_keyMapping.set(GamepadVirtualKeyCode::X, GamepadKeyCode::BUTTON_1 + 2);
			_keyMapping.set(GamepadVirtualKeyCode::Y, GamepadKeyCode::BUTTON_1 + 3);
			_keyMapping.set(GamepadVirtualKeyCode::L_SHOULDER, GamepadKeyCode::BUTTON_1 + 4);
			_keyMapping.set(GamepadVirtualKeyCode::R_SHOULDER, GamepadKeyCode::BUTTON_1 + 5);
			_keyMapping.set(GamepadVirtualKeyCode::SELECT, GamepadKeyCode::BUTTON_1 + 6);
			_keyMapping.set(GamepadVirtualKeyCode::START, GamepadKeyCode::BUTTON_1 + 7);
			_keyMapping.set(GamepadVirtualKeyCode::L_THUMB, GamepadKeyCode::BUTTON_1 + 8);
			_keyMapping.set(GamepadVirtualKeyCode::R_THUMB, GamepadKeyCode::BUTTON_1 + 9);
		}

		_keyMapping.undefinedCompletion(MAX_AXES, MAX_BUTTONS);
	}

	void Gamepad::_setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone) {
		if (deadZone) {
			auto& dzVal = *deadZone;

			Math::clamp(dzVal.data, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>);

			if (dzVal[1] < dzVal[0]) {
				auto tmp = dzVal[0];
				dzVal[0] = dzVal[1];
				dzVal[1] = tmp;
			}

			std::scoped_lock lock(_deadZoneMutex);

			_deadZone.insert_or_assign(keyCode, dzVal);
		} else {
			std::scoped_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(keyCode); itr != _deadZone.end()) _deadZone.erase(itr);
		}
	}

	/*
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
	*/

	DeviceState::CountType Gamepad::_getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
		auto axes = &_state.lX;
		GamepadKeyCode mappingVals[2];
		_keyMapping.get(stickX, 2, mappingVals);

		return translate(
			_normalizeStick(_readAxisVal(axes, mappingVals[0], (std::numeric_limits<int16_t>::max)())),
			_normalizeStick(_readAxisVal(axes, mappingVals[1], (std::numeric_limits<int16_t>::max)())),
			_getDeadZone(key), data, count);
	}

	DeviceState::CountType Gamepad::_getCombinedTrigger(GamepadKeyCode k, GamepadVirtualKeyCode vk, uint8_t index, DeviceStateValue& data) const {
		DeviceStateValue values[2];
		_normalizeCombinedAxis(_readAxisVal(&_state.lX, k, (std::numeric_limits<int16_t>::max)()), values[1], values[0]);
		data = translate(values[index], _getDeadZone(vk));

		return 1;
	}

	DeviceState::CountType Gamepad::_getAxis(GamepadKeyCode k, GamepadVirtualKeyCode vk, DeviceStateValue& data) const {
		data = translate(_normalizeAxis(_readAxisVal(&_state.lX, k, 0)), _getDeadZone(vk));
		return 1;
	}

	void Gamepad::_dispatchStick(LONG oldX, LONG oldY, LONG newX, LONG newY, GamepadVirtualKeyCode key) const {
		//if (oldX == newX && oldY == newY) return;

		auto dz = _getDeadZone(key);

		DeviceStateValue oldDzVals[2], newDzVals[2];
		translate(_normalizeStick(oldX), _normalizeStick(oldY), dz, oldDzVals, 2);
		translate(_normalizeStick(newX), _normalizeStick(newY), dz, newDzVals, 2);

		if (oldDzVals[0] != newDzVals[0] || oldDzVals[1] != newDzVals[1]) {
			DeviceState ds = { (DeviceState::CodeType)key, 2, newDzVals };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void Gamepad::_dispatchCombinedTrigger(LONG oldVal, LONG newVal) const {
		//if (oldVal == newVal) return;

		DeviceStateValue oldVals[2], newVals[2];
		_normalizeCombinedAxis(oldVal, oldVals[1], oldVals[0]);
		_normalizeCombinedAxis(newVal, newVals[1], newVals[0]);
		auto ldz = _getDeadZone(GamepadVirtualKeyCode::L_TRIGGER);
		auto rdz = _getDeadZone(GamepadVirtualKeyCode::R_TRIGGER);

		if (auto newDzVal = translate(newVals[0], ldz); newDzVal != translate(oldVals[0], ldz)) {
			DeviceState ds = { (DeviceState::CodeType)GamepadVirtualKeyCode::L_TRIGGER, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}

		if (auto newDzVal = translate(newVals[1], rdz); newDzVal != translate(oldVals[1], rdz)) {
			DeviceState ds = { (DeviceState::CodeType)GamepadVirtualKeyCode::R_TRIGGER, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void Gamepad::_dispatchAxis(LONG oldVal, LONG newVal, GamepadVirtualKeyCode key) const {
		//if (oldVal == newVal) return;

		auto dz = _getDeadZone(key);
		if (auto newDzVal = translate(_normalizeAxis(newVal), dz); newDzVal != translate(_normalizeAxis(oldVal), dz)) {
			DeviceState ds = { (DeviceState::CodeType)key, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}
}