#include "GamepadDS4.h"
#include "aurora/Time.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	GamepadDS4::GamepadDS4(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid),
		_isBluetooth(false),
		_outputDirty(false),
		_inputBufferOffset(0),
		_outputBufferOffset(0) {
		using namespace aurora::extensions;

		_setKeyMapping(nullptr);

		uint8_t buf[65];
		do {
			if (auto rst = HID::read(*_hid, buf, sizeof(buf), HID::IN_TIMEOUT_BLOCKING); HID::isSuccess(rst)) {
				_isBluetooth = rst > 64;
				break;
			} else if (rst == HID::OUT_ERROR) {
				break;
			}
		} while (true);

		memset(_outputState, 0, sizeof(OutputStateBuffer));
		if (_isBluetooth) {
			_outputState[0] = 0x11;
			_outputState[1] = 0x80;
			_outputState[3] = 0xFF;

			_inputBufferOffset = 3;
			_outputBufferOffset = 6;
		} else {
			_outputState[0] = 0x05;
			_outputState[1] = 0xFF;

			_inputBufferOffset = 1;
			_outputBufferOffset = 4;
		}

		memcpy(_inputState, buf + _inputBufferOffset, sizeof(_inputState));
		memset(_outputBuffer, 0, sizeof(_outputBuffer));

		auto time = Time::now();
		for (size_t i = 0; i < 2; ++i) {
			auto& state = _touchStateValues[i];
			state.time = time;
		}
		_translateTouch(buf + _inputBufferOffset + (std::underlying_type_t<InputBufferOffset>)InputBufferOffset::TOUCHES, _touchStateValues, time);
	}

	DeviceState::CountType GamepadDS4::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		using namespace aurora::enum_operators;

		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				std::shared_lock lock(_inputStateMutex);

				switch ((GamepadVirtualKeyCode)code) {
				case GamepadVirtualKeyCode::L_STICK:
					return _getStick(GamepadVirtualKeyCode::L_STICK_X, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::R_STICK:
					return _getStick(GamepadVirtualKeyCode::R_STICK_X, (GamepadVirtualKeyCode)code, (DeviceStateValue*)values, count);
				case GamepadVirtualKeyCode::DPAD:
					return _getDPad(_inputState[(int32_t)InputBufferOffset::D_PAD], (DeviceStateValue*)values);
				default:
				{
					if (code >= GamepadVirtualKeyCode::SEPARATE_AXIS_START && code <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
						((DeviceStateValue*)values)[0] = translate(_normalizeAxis(_readAxisVal(_inputState, _keyMapping.get((GamepadVirtualKeyCode)code), 0)), _getDeadZone((GamepadVirtualKeyCode)code));
						return 1;
					} else if (code >= GamepadVirtualKeyCode::BUTTON_START && code <= GamepadVirtualKeyCode::BUTTON_END) {
						((DeviceStateValue*)values)[0] = _translateButton(_readButtonVal(_inputState, _keyMapping.get((GamepadVirtualKeyCode)code)));
						return 1;
					}

					return 0;
				}
				}
			}

			return 0;
		}
		case DeviceStateType::TOUCH:
		{
			if (values && count) {
				auto time = Time::now();
				DeviceState::CountType c = 0;

				std::shared_lock lock(_inputStateMutex);

				for (size_t i = 0; i < 2; ++i) {
					auto& state = _touchStateValues[i];
					if (state.phase != DeviceTouchPhase::END) {
						_getTouch(state, time, ((DeviceTouchStateValue*)values)[c++]);
						if (c == count) break;
					}
				}

				return c;
			}

			return 0;
		}
		case DeviceStateType::TOUCH_RESOLUTION:
		{
			if (values && count) {
				DeviceState::CountType c = 1;

				((DeviceStateValue*)values)[0] = TOUCH_PAD_RESOLUTION_X;
				if (count > 1) {
					((DeviceStateValue*)values)[1] = TOUCH_PAD_RESOLUTION_Y;
					++c;
				}

				return c;
			}

			return 0;
		}
		default:
			return GamepadBaseType::getState(type, code, values, count);
		}
	}

	DeviceState::CountType GamepadDS4::setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::KEY_MAPPING:
		{
			if (!count) values = nullptr;

			{
				std::scoped_lock lock(_inputStateMutex);

				_setKeyMapping((const GamepadKeyMapping*)values);
			}

			return 1;
		}
		case DeviceStateType::VIBRATION:
		{
			if (values && count) {
				DeviceState::CodeType c = 1;
				DeviceStateValue l = ((DeviceStateValue*)values)[0], r;
				if (count >= 2) {
					r = ((DeviceStateValue*)values)[1];
					++c;
				} else {
					r = Math::ZERO<DeviceStateValue>;
				}

				_setVibration(l, r);
				return c;
			}

			return 0;
		}
		case DeviceStateType::LED:
		{
			if (values && count) {
				DeviceState::CodeType c = 1;
				DeviceStateValue r = ((DeviceStateValue*)values)[0], g, b;
				if (count >= 2) {
					g = ((DeviceStateValue*)values)[1];
					++c;
					if (count >= 3) {
						b = ((DeviceStateValue*)values)[2];
						++c;
					}
				}

				_setLed(r, g, b);
				return c;
			}

			return 0;
		}
		default:
			return GamepadBaseType::setState(type, code, values, count);
		}
	}

	void GamepadDS4::_doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) {
		using namespace aurora::enum_operators;

		auto newState = inputBuffer + _inputBufferOffset;
		inputBufferSize -= _inputBufferOffset;

		auto time = Time::now();
		
		if (inputBufferSize > sizeof(InputStateBuffer)) inputBufferSize = sizeof(InputStateBuffer);

		if (!dispatchEvent) {
			std::scoped_lock lock(_inputStateMutex);

			memcpy(_inputState, newState, inputBufferSize);

			_translateTouch(newState + (std::underlying_type_t<InputBufferOffset>)InputBufferOffset::TOUCHES, _touchStateValues, time);

			return;
		}

		InputStateBuffer oldState;
		GamepadKeyMapping keyMapping(NO_INIT);
		InternalDeviceTouchStateValue touchStateValues[2];
		bool changedTouchStates[] = { false, false };
		DeviceState::CountType changedTouchCount = 0;
		{
			std::scoped_lock lock(_inputStateMutex);

			keyMapping = _keyMapping;
			memcpy(oldState, _inputState, sizeof(_inputState));
			memcpy(_inputState, newState, sizeof(_inputState));

			for (size_t i = 0; i < 2; ++i) touchStateValues[i] = _touchStateValues[i];
			_translateTouch(newState + (std::underlying_type_t<InputBufferOffset>)InputBufferOffset::TOUCHES, touchStateValues, time);
			for (size_t i = 0; i < 2; ++i) {
				if (!touchStateValues[i].isSame(_touchStateValues[i])) {
					if (_touchStateValues[i].phase != DeviceTouchPhase::END || touchStateValues[i].phase != DeviceTouchPhase::END || _touchStateValues[i].fingerID != 0) {
						changedTouchStates[i] = true;
						++changedTouchCount;
					}

					_touchStateValues[i] = touchStateValues[i];
				} else {
					_touchStateValues[i].deltaTime = touchStateValues[i].deltaTime;
				}
			}
		}

		GamepadKeyCode mappingVals[4];
		keyMapping.get(GamepadVirtualKeyCode::L_STICK_X, 4, mappingVals);
		for (size_t i = 0; i < 2; ++i) {
			auto idx = i << 1;
			_dispatchStick(
				_readAxisVal(oldState, mappingVals[idx], (std::numeric_limits<int8_t>::max)()),
				_readAxisVal(oldState, mappingVals[idx + 1], (std::numeric_limits<int8_t>::max)()),
				_readAxisVal(newState, mappingVals[idx], (std::numeric_limits<int8_t>::max)()),
				_readAxisVal(newState, mappingVals[idx + 1], (std::numeric_limits<int8_t>::max)()),
				GamepadVirtualKeyCode::L_STICK + i);
		}

		keyMapping.forEach([&](GamepadVirtualKeyCode vk, GamepadKeyCode k) {
			if (vk >= GamepadVirtualKeyCode::L_TRIGGER && vk <= GamepadVirtualKeyCode::AXIS_END) {
				_dispatchAxis(_readAxisVal(oldState, k, 0), _readAxisVal(newState, k, 0), vk);
			} else if (vk >= GamepadVirtualKeyCode::BUTTON_START && vk <= GamepadVirtualKeyCode::BUTTON_END) {
				if (auto newVal = _readButtonVal(newState, k); newVal != _readButtonVal(oldState, k)) {
					auto value = _translateButton(newVal);
					DeviceState ds = { (DeviceState::CodeType)vk, 1, &value };
					_eventDispatcher->dispatchEvent(this, value > Math::ZERO<DeviceStateValue> ? DeviceEvent::DOWN : DeviceEvent::UP, &ds);
				}
			}
		});

		_dispatchDPad(oldState[(size_t)InputBufferOffset::D_PAD], newState[(size_t)InputBufferOffset::D_PAD]);

		if (changedTouchCount) {
			DeviceTouchStateValue touches[2];
			DeviceState::CountType n = 0;
			for (size_t i = 0; i < 2; ++i) {
				if (changedTouchStates[i]) _getTouch(touchStateValues[i], time, touches[n++]);
			}
			DeviceState k = { 0, changedTouchCount, touches };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::TOUCH, &k);
		}
		{
			/*
			auto i = (std::underlying_type_t<InputBufferOffset>)InputBufferOffset::GYRO_X;
			i = 12;

			auto h_x = buf[i];
			auto l_x = buf[i + 1];

			auto h_y = buf[i + 2];
			auto l_y = buf[i + 3];

			auto h_z = buf[i + 4];
			auto l_z = buf[i + 5];

			int16_t gyro_x = h_x << 8 | l_x;
			int16_t gyro_y = h_y << 8 | l_y;
			int16_t gyro_z = h_z << 8 | l_z;

			//printdln(gyro_x, "  ", gyro_y, "  ", gyro_z);

			int a = 1;
			*/
		}
	}

	bool GamepadDS4::_doOutput() {
		if (_outputDirty.exchange(false)) {
			std::shared_lock lock(_outputBufferMutex);

			memcpy(_outputState + _outputBufferOffset, _outputBuffer, sizeof(OutputBuffer));

			return true;
		}

		return false;
	}

	void GamepadDS4::_setKeyMapping(const GamepadKeyMapping* mapping) {
		using namespace aurora::enum_operators;

		if (mapping) {
			_keyMapping = *mapping;
		} else {
			_keyMapping.clear();

			_keyMapping.set(GamepadVirtualKeyCode::L_STICK_X, GamepadKeyCode::AXIS_1);
			_keyMapping.set(GamepadVirtualKeyCode::L_STICK_Y, GamepadKeyCode::AXIS_1 + 1);
			_keyMapping.set(GamepadVirtualKeyCode::R_STICK_X, GamepadKeyCode::AXIS_1 + 2);
			_keyMapping.set(GamepadVirtualKeyCode::R_STICK_Y, GamepadKeyCode::AXIS_1 + 5);
			_keyMapping.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 3);
			_keyMapping.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 4);

			_keyMapping.set(GamepadVirtualKeyCode::SQUARE, GamepadKeyCode::BUTTON_1);
			_keyMapping.set(GamepadVirtualKeyCode::CROSS, GamepadKeyCode::BUTTON_1 + 1);
			_keyMapping.set(GamepadVirtualKeyCode::CIRCLE, GamepadKeyCode::BUTTON_1 + 2);
			_keyMapping.set(GamepadVirtualKeyCode::TRIANGLE, GamepadKeyCode::BUTTON_1 + 3);
			_keyMapping.set(GamepadVirtualKeyCode::L1, GamepadKeyCode::BUTTON_1 + 4);
			_keyMapping.set(GamepadVirtualKeyCode::R1, GamepadKeyCode::BUTTON_1 + 5);
			_keyMapping.set(GamepadVirtualKeyCode::L2_BUTTON, GamepadKeyCode::BUTTON_1 + 6);
			_keyMapping.set(GamepadVirtualKeyCode::R2_BUTTON, GamepadKeyCode::BUTTON_1 + 7);
			_keyMapping.set(GamepadVirtualKeyCode::SHARE, GamepadKeyCode::BUTTON_1 + 8);
			_keyMapping.set(GamepadVirtualKeyCode::OPTIONS, GamepadKeyCode::BUTTON_1 + 9);
			_keyMapping.set(GamepadVirtualKeyCode::L3, GamepadKeyCode::BUTTON_1 + 10);
			_keyMapping.set(GamepadVirtualKeyCode::R3, GamepadKeyCode::BUTTON_1 + 11);
			_keyMapping.set(GamepadVirtualKeyCode::UNDEFINED_BUTTON_1, GamepadKeyCode::BUTTON_1 + 12);
			_keyMapping.set(GamepadVirtualKeyCode::TOUCH_PAD, GamepadKeyCode::BUTTON_1 + 13);
		}

		_keyMapping.undefinedCompletion(MAX_AXES, MAX_BUTTONS);
	}

	uint8_t GamepadDS4::_readAxisVal(const uint8_t* state, GamepadKeyCode k, uint8_t defaultVal) {
		using namespace aurora::enum_operators;

		switch (k) {
		case GamepadKeyCode::AXIS_1:
		case GamepadKeyCode::AXIS_1 + 1:
		case GamepadKeyCode::AXIS_1 + 2:
			return state[(size_t)(k - GamepadKeyCode::AXIS_1)];
		case GamepadKeyCode::AXIS_1 + 3:
		case GamepadKeyCode::AXIS_1 + 4:
			return state[(size_t)InputBufferOffset::L_TRIGGER + (size_t)(k - GamepadKeyCode::AXIS_1 - 3)];
		case GamepadKeyCode::AXIS_1 + 5:
			return state[(size_t)InputBufferOffset::RY];
		default:
			return defaultVal;
		}
	}

	DeviceState::CountType GamepadDS4::_getButton(uint8_t state, InputMask mask, DeviceStateValue* data) {
		auto val = state & (uint8_t)mask;
		data[0] = val ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		return 1;
	}

	DeviceState::CountType GamepadDS4::_getDPad(uint8_t state, DeviceStateValue* data) {
		if (auto val = state & 0xF; val <= 7) {
			data[0] = val * Math::PI_4<DeviceStateValue>;
		} else {
			data[0] = Math::NEGATIVE_ONE<DeviceStateValue>;
		}
		return 1;
	}

	DeviceState::CountType GamepadDS4::_getStick(GamepadVirtualKeyCode stickX, GamepadVirtualKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
		GamepadKeyCode mappingVals[2];
		_keyMapping.get(stickX, 2, mappingVals);

		return translate(
			_normalizeStick(_readAxisVal(_inputState, mappingVals[0], (std::numeric_limits<int16_t>::max)())),
			_normalizeStick(_readAxisVal(_inputState, mappingVals[1], (std::numeric_limits<int16_t>::max)())),
			_getDeadZone(key), data, count);
	}

	void GamepadDS4::_getTouch(const InternalDeviceTouchStateValue& in, size_t time, DeviceTouchStateValue& out) {
		out = in;
		out.deltaTime += time - in.time;
	}

	void GamepadDS4::_setVibration(DeviceStateValue left, DeviceStateValue right) {
		auto l = (uint8_t)(Math::clamp(left, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * NUMBER_255<DeviceStateValue>);
		auto r = (uint8_t)(Math::clamp(right, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * NUMBER_255<DeviceStateValue>);

		std::scoped_lock lock(_outputBufferMutex);

		_outputBuffer[0] = l;
		_outputBuffer[1] = r;

		_outputDirty = true;
	}

	void GamepadDS4::_setLed(DeviceStateValue red, DeviceStateValue green, DeviceStateValue blue) {
		auto r = (uint8_t)(Math::clamp(red, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * NUMBER_255<DeviceStateValue>);
		auto g = (uint8_t)(Math::clamp(green, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * NUMBER_255<DeviceStateValue>);
		auto b = (uint8_t)(Math::clamp(blue, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * NUMBER_255<DeviceStateValue>);

		std::scoped_lock lock(_outputBufferMutex);

		_outputBuffer[2] = r;
		_outputBuffer[3] = g;
		_outputBuffer[4] = b;

		_outputDirty = true;
	}

	void GamepadDS4::_dispatchButton(uint8_t oldState, uint8_t newState, InputMask mask, GamepadVirtualKeyCode key) {
		auto oldVal = oldState & (uint8_t)mask;
		auto newVal = newState & (uint8_t)mask;
		if (oldVal != newVal) {
			auto value = newVal ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher->dispatchEvent(this, newVal ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}

	void GamepadDS4::_dispatchDPad(uint8_t oldState, uint8_t newState) {
		auto oldVal = oldState & 0xF;
		auto newVal = newState & 0xF;
		if (oldVal != newVal) {
			if (newVal <= 7) {
				auto value = newVal * Math::PI_4<DeviceStateValue>;
				DeviceState k = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &value };
				_eventDispatcher->dispatchEvent(this, DeviceEvent::DOWN, &k);
			} else if (oldVal <= 7) {
				auto value = Math::NEGATIVE_ONE<DeviceStateValue>;
				DeviceState k = { (DeviceState::CodeType)GamepadVirtualKeyCode::DPAD, 1, &value };
				_eventDispatcher->dispatchEvent(this, DeviceEvent::UP, &k);
			}
		}
	}

	void GamepadDS4::_dispatchStick(uint8_t oldX, uint8_t oldY, uint8_t newX, uint8_t newY, GamepadVirtualKeyCode key) {
		auto dz = _getDeadZone(key);

		DeviceStateValue oldDzVals[2], newDzVals[2];
		translate(_normalizeStick(oldX), _normalizeStick(oldY), dz, oldDzVals, 2);
		translate(_normalizeStick(newX), _normalizeStick(newY), dz, newDzVals, 2);

		if (oldDzVals[0] != newDzVals[0] || oldDzVals[1] != newDzVals[1]) {
			DeviceState ds = { (DeviceState::CodeType)key, 2, newDzVals };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void GamepadDS4::_dispatchAxis(uint8_t oldVal, uint8_t newVal, GamepadVirtualKeyCode key) {
		auto dz = _getDeadZone(key);
		if (auto newDzVal = translate(_normalizeAxis(newVal), dz); newDzVal != translate(_normalizeAxis(oldVal), dz)) {
			DeviceState ds = { (DeviceState::CodeType)key, 1, &newDzVal };
			_eventDispatcher->dispatchEvent((void*)this, DeviceEvent::MOVE, &ds);
		}
	}

	void GamepadDS4::_translateTouch(uint8_t* data, InternalDeviceTouchStateValue* states, size_t time) {
		if (data[0]) {
			size_t offset = 2;

			for (size_t i = 0; i < 2; ++i) {
				auto& state = states[i];

				auto isTouch = (data[offset] >> 7 & 0b1) == 0;
				auto id = data[offset] & 0x7F;
				auto x = (((data[offset + 2] & 0xF) << 8) | (data[offset + 1])) * Math::RECIPROCAL<TOUCH_PAD_MAX_X>;
				auto y = ((data[offset + 3] << 4) | (data[offset + 2] >> 4 & 0xF)) * Math::RECIPROCAL<TOUCH_PAD_MAX_Y>;

				state.code = 0;
				if (state.fingerID == id) {
					state.deltaPhase = state.phase;
					if (isTouch) {
						state.phase = state.phase == DeviceTouchPhase::END ? DeviceTouchPhase::BEGIN : DeviceTouchPhase::MOVE;
					} else {
						state.phase = DeviceTouchPhase::END;
					}
					state.deltaPosition.set(x - state.position[0], y - state.position[1]);
					state.deltaTime = time - state.time;
				} else {
					state.fingerID = id;
					state.phase = isTouch ? DeviceTouchPhase::BEGIN : DeviceTouchPhase::END;
					state.deltaPhase = DeviceTouchPhase::END;
					state.deltaPosition.set(Math::ZERO<DeviceStateValue>);
					state.deltaTime = Math::ZERO<DeviceStateValue>;
				}
				state.position.set(x, y);
				state.time = time;

				offset += 4;
			}
		} else {
			for (size_t i = 0; i < 2; ++i) {
				auto& state = states[i];

				state.code = 0;
				state.deltaPhase = state.phase;
				state.phase = DeviceTouchPhase::END;
				state.deltaPosition.set(Math::ZERO<DeviceStateValue>);
				state.deltaTime = time - state.time;
				state.time = time;
			}
		}
	}
}