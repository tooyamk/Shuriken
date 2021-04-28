#include "Gamepad.h"
#include "Input.h"
#include "aurora/math/Math.h"

namespace aurora::modules::inputs::xinput {
	Gamepad::Gamepad(Input& input, const DeviceInfo& info) :
		_index(((InternalGUID&)*info.guid.getData()).index - 1),
		_input(input),
		_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
		_info(info) {

		_setKeyMapping(nullptr);

		Vec2<DeviceStateValue> dz(Math::ZERO<DeviceStateValue>, Math::TWENTIETH<DeviceStateValue>);
		_setDeadZone(GamepadVirtualKeyCode::L_STICK, &dz);
		_setDeadZone(GamepadVirtualKeyCode::R_STICK, &dz);
		_setDeadZone(GamepadVirtualKeyCode::L_TRIGGER, &dz);
		_setDeadZone(GamepadVirtualKeyCode::R_TRIGGER, &dz);

		_readState(_state);
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
				case GamepadVirtualKeyCode::DPAD:
					((DeviceStateValue*)values)[0] = _translateDpad(_state.Gamepad.wButtons);
					return 1;
				default:
				{
					if (code >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && code <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
						((DeviceStateValue*)values)[0] = translate(_normalizeAxis(_readAxisVal(_state.Gamepad, _keyMapping.get((GamepadVirtualKeyCode)code), 0)), _getDeadZone((GamepadVirtualKeyCode)code));
						return 1;
					} else if (code >= GamepadVirtualKeyCode::BUTTON_START && code <= GamepadVirtualKeyCode::BUTTON_END) {
						((DeviceStateValue*)values)[0] = _translateButton(_readButtonVal(_state.Gamepad.wButtons, _keyMapping.get((GamepadVirtualKeyCode)code)));
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

	DeviceState::CountType Gamepad::setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) {
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
		using namespace aurora::enum_operators;

		XINPUT_STATE newState;
		if (!_readState(newState)) return;

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(&_state, &newState, sizeof(XINPUT_STATE));

			return;
		}

		XINPUT_STATE oldState;
		GamepadKeyMapping keyMapping(NO_INIT);
		{
			std::scoped_lock lock(_mutex);

			keyMapping = _keyMapping;
			memcpy(&oldState, &_state, sizeof(_state));
			memcpy(&_state, &newState, sizeof(_state));
		}

		auto& oldGamepad = oldState.Gamepad;
		auto oldBtns = oldGamepad.wButtons;
		auto& newGamepad = newState.Gamepad;
		auto newBtns = newGamepad.wButtons;

		GamepadKeyCode mappingVals[4];
		keyMapping.get(GamepadVirtualKeyCode::L_STICK_X, 4, mappingVals);
		for (size_t i = 0; i < 2; ++i) {
			auto idx = i << 1;
			_dispatchStick(
				_readAxisVal(oldGamepad, mappingVals[idx], (std::numeric_limits<int16_t>::max)()), 
				_readAxisVal(oldGamepad, mappingVals[idx + 1], (std::numeric_limits<int16_t>::max)()),
				_readAxisVal(newGamepad, mappingVals[idx], (std::numeric_limits<int16_t>::max)()),
				_readAxisVal(newGamepad, mappingVals[idx + 1], (std::numeric_limits<int16_t>::max)()),
				GamepadVirtualKeyCode::L_STICK + i);
		}

		keyMapping.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCode k) {
			if (vk >= GamepadVirtualKeyCode::L_TRIGGER && vk <= GamepadVirtualKeyCode::AXIS_END) {
				_dispatchAxis(_readAxisVal(oldGamepad, k, 0), _readAxisVal(newGamepad, k, 0), vk);
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				if (auto newVal = _readButtonVal(newBtns, k); newVal != _readButtonVal(oldBtns, k)) {
					auto value = _translateButton(newVal);
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &value };
					_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
				}
			}
		});

		if ((oldBtns & XINPUT_GAMEPAD_DPAD_UP) != (newBtns & XINPUT_GAMEPAD_DPAD_UP) ||
			(oldBtns & XINPUT_GAMEPAD_DPAD_RIGHT) != (newBtns & XINPUT_GAMEPAD_DPAD_RIGHT) ||
			(oldBtns & XINPUT_GAMEPAD_DPAD_DOWN) != (newBtns & XINPUT_GAMEPAD_DPAD_DOWN) ||
			(oldBtns & XINPUT_GAMEPAD_DPAD_LEFT) != (newBtns & XINPUT_GAMEPAD_DPAD_LEFT)) {
			auto value = _translateDpad(newGamepad.wButtons);
			DeviceState ds = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &value };
			_eventDispatcher->dispatchEvent(this, value >= Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
		}
	}

	bool Gamepad::_readState(XINPUT_STATE& state) {
		return XInputGetState(_index, &state) == ERROR_SUCCESS;
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
			_keyMapping.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 5);

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

	void Gamepad::_setVibration(DeviceStateValue left, DeviceStateValue right) {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = Math::clamp(left, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * (std::numeric_limits<uint16_t>::max)();
		vibration.wRightMotorSpeed = Math::clamp(right, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * (std::numeric_limits<uint16_t>::max)();
		XInputSetState(_index, &vibration);
	}

	uint16_t Gamepad::_readAxisVal(const XINPUT_GAMEPAD& gamepad, GamepadKeyCode k, uint16_t defaultVal) {
		using namespace aurora::enum_operators;

		switch (k) {
		case GamepadKeyCode::AXIS_1:
			return _traslateRawThumbX(gamepad.sThumbLX);
		case GamepadKeyCode::AXIS_1 + 1:
			return _traslateRawThumbY(gamepad.sThumbLY);
		case GamepadKeyCode::AXIS_1 + 2:
			return _traslateRawTrigger(gamepad.bLeftTrigger);
		case GamepadKeyCode::AXIS_1 + 3:
			return _traslateRawThumbX(gamepad.sThumbRX);
		case GamepadKeyCode::AXIS_1 + 4:
			return _traslateRawThumbY(gamepad.sThumbRY);
		case GamepadKeyCode::AXIS_1 + 5:
			return _traslateRawTrigger(gamepad.bRightTrigger);
		default:
			return defaultVal;
		}
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

	DeviceState::CountType Gamepad::_getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
		auto& gamepad = _state.Gamepad;
		GamepadKeyCode mappingVals[2];
		_keyMapping.get(stickX, 2, mappingVals);

		return translate(
			_normalizeStick(_readAxisVal(gamepad, mappingVals[0], (std::numeric_limits<int16_t>::max)())),
			_normalizeStick(_readAxisVal(gamepad, mappingVals[1], (std::numeric_limits<int16_t>::max)())),
			_getDeadZone(key), data, count);
	}

	void Gamepad::_dispatchStick(uint16_t oldX, uint16_t oldY, uint16_t newX, uint16_t newY, GamepadVirtualKeyCode key) {
		auto dz = _getDeadZone(key);

		DeviceStateValue oldDzVals[2], newDzVals[2];
		translate(_normalizeStick(oldX), _normalizeStick(oldY), dz, oldDzVals, 2);
		translate(_normalizeStick(newX), _normalizeStick(newY), dz, newDzVals, 2);

		if (oldDzVals[0] != newDzVals[0] || oldDzVals[1] != newDzVals[1]) {
			DeviceState ds = { (DeviceState::CodeType)key, 2, newDzVals };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void Gamepad::_dispatchAxis(uint16_t oldVal, uint16_t newVal, GamepadVirtualKeyCode key) {
		auto dz = _getDeadZone(key);
		if (auto newDzVal = translate(_normalizeAxis(newVal), dz); newDzVal != translate(_normalizeAxis(oldVal), dz)) {
			DeviceState ds = { (DeviceState::CodeType)key, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}
}