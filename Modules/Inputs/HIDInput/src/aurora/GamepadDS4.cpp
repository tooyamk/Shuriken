#include "GamepadDS4.h"
#include "aurora/Time.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	GamepadDS4::GamepadDS4(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid),
		_isBluetooth(false),
		_outputDirty(false),
		_outputOffset(0) {
		using namespace aurora::enum_operators;
		using namespace aurora::extensions;

		for (size_t i = 0; i < 4; ++ i) _inputState[(std::underlying_type_t<InputBufferOffset>)(InputBufferOffset::LX + i)] = 0x80;
		_inputState[(std::underlying_type_t<InputBufferOffset>)InputBufferOffset::D_PAD] = 0x8;

		uint8_t buf[65];
		do {
			auto rst = HID::read(*_hid, buf, sizeof(buf), HID::IN_TIMEOUT_BLOCKING);
			if (HID::isSuccess(rst)) {
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

			_outputOffset = 6;
		} else {
			_outputState[0] = 0x05;
			_outputState[1] = 0xFF;

			_outputOffset = 4;
		}

		memset(_outputBuffer, 0, sizeof(OutputBuffer));

		auto time = Time::now();
		for (size_t i = 0; i < 2; ++i) {
			auto& state = _touchStateValues[i];

			state.code = 0;
			state.fingerID = 0;
			state.phase = DeviceTouchPhase::END;
			state.deltaPhase = state.phase;
			state.time = time;
			state.deltaTime = Math::ZERO<decltype(state.deltaTime)>;
		}
	}

	DeviceState::CountType GamepadDS4::getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (values && count) {
				std::shared_lock lock(_inputStateMutex);

				switch ((GamepadKeyCode)code) {
				case GamepadKeyCode::DPAD:
					return _getDPad(_inputState[(int32_t)InputBufferOffset::D_PAD], (DeviceStateValue*)values);
				case GamepadKeyCode::SQUARE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::SQUARE], InputMask::SQUARE, (DeviceStateValue*)values);
				case GamepadKeyCode::CROSS:
					return _getButton(_inputState[(int32_t)InputBufferOffset::CROSS], InputMask::CROSS, (DeviceStateValue*)values);
				case GamepadKeyCode::CIRCLE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::CIRCLE], InputMask::CIRCLE, (DeviceStateValue*)values);
				case GamepadKeyCode::TRIANGLE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::TRIANGLE], InputMask::TRIANGLE, (DeviceStateValue*)values);
				case GamepadKeyCode::L1:
					return _getButton(_inputState[(int32_t)InputBufferOffset::L1], InputMask::L1, (DeviceStateValue*)values);
				case GamepadKeyCode::R1:
					return _getButton(_inputState[(int32_t)InputBufferOffset::R1], InputMask::R1, (DeviceStateValue*)values);
				case GamepadKeyCode::SHARE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::SHARE], InputMask::SHARE, (DeviceStateValue*)values);
				case GamepadKeyCode::OPTIONS:
					return _getButton(_inputState[(int32_t)InputBufferOffset::OPTIONS], InputMask::OPTIONS, (DeviceStateValue*)values);
				case GamepadKeyCode::L3:
					return _getButton(_inputState[(int32_t)InputBufferOffset::L3], InputMask::L3, (DeviceStateValue*)values);
				case GamepadKeyCode::R3:
					return _getButton(_inputState[(int32_t)InputBufferOffset::R3], InputMask::R3, (DeviceStateValue*)values);
				case GamepadKeyCode::TOUCH_PAD:
					return _getButton(_inputState[(int32_t)InputBufferOffset::TOUTCH_PAD], InputMask::TOUTCH_PAD_CLICK, (DeviceStateValue*)values);
				case GamepadKeyCode::L2:
					return _getTrigger(_inputState[(int32_t)InputBufferOffset::L2], GamepadKeyCode::L2, (DeviceStateValue*)values);
				case GamepadKeyCode::R2:
					return _getTrigger(_inputState[(int32_t)InputBufferOffset::R2], GamepadKeyCode::R2, (DeviceStateValue*)values);
				case GamepadKeyCode::L_STICK:
					return _getStick(_inputState[(int32_t)InputBufferOffset::LX], _inputState[(int32_t)InputBufferOffset::LY], GamepadKeyCode::L_STICK, (DeviceStateValue*)values, count);
				case GamepadKeyCode::R_STICK:
					return _getStick(_inputState[(int32_t)InputBufferOffset::RX], _inputState[(int32_t)InputBufferOffset::RY], GamepadKeyCode::R_STICK, (DeviceStateValue*)values, count);
				default:
					return 0;
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
		default:
			return 0;
		}
	}

	DeviceState::CountType GamepadDS4::setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) {
		switch (type) {
		case DeviceStateType::DEAD_ZONE:
		{
			if (values && count) {
				_setDeadZone((GamepadKeyCode)code, ((DeviceStateValue*)values)[0]);
				return 1;
			}

			return 0;
		}
		case DeviceStateType::VIBRATION:
		{
			if (values && count) {
				DeviceState::CodeType c = 1;
				DeviceStateValue l = ((DeviceStateValue*)values)[0], r;
				if (count >= 2) {
					r = ((DeviceStateValue*)values)[1];
					++c;
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
			return 0;
		}
	}

	void GamepadDS4::_doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) {
		using namespace aurora::enum_operators;

		auto buf = inputBuffer;

		if (_isBluetooth) {
			if (buf[0] != 0x11) return;
			buf += 3;
			inputBufferSize -= 3;
		} else {
			if (buf[0] != 0x1) return;
			++buf;
			--inputBufferSize;
		}

		auto time = Time::now();
		
		if (inputBufferSize > sizeof(InputStateBuffer)) inputBufferSize = sizeof(InputStateBuffer);

		if (!dispatchEvent) {
			std::scoped_lock lock(_inputStateMutex);

			memcpy(_inputState, buf, inputBufferSize);

			_translateTouch(buf + (std::underlying_type_t<InputBufferOffset>)InputBufferOffset::TOUCHES, _touchStateValues, time);

			return;
		}

		uint16_t oldSticks[2];
		InputStateBuffer changedIndices;
		InputStateBuffer changedOldState;
		uint8_t changedLen = 0;

		InternalDeviceTouchStateValue touchStateValues[2];
		{
			std::shared_lock lock(_inputStateMutex);

			for (size_t i = 0; i < 2; ++i) touchStateValues[i] = _touchStateValues[i];
		}
		_translateTouch(buf + (std::underlying_type_t<InputBufferOffset>)InputBufferOffset::TOUCHES, touchStateValues, time);
		bool changedTouchStates[] = { false, false };
		DeviceState::CountType changedTouchCount = 0;

		auto oldBuf16 = (uint16_t*)_inputState;
		auto curBuf16 = (uint16_t*)buf;

		{
			std::scoped_lock lock(_inputStateMutex);

			for (size_t i = 0; i < 2; ++i) {
				oldSticks[i] = oldBuf16[i];
				oldBuf16[i] = curBuf16[i];
			}

			for (uint8_t i = sizeof(oldSticks); i < inputBufferSize; ++i) {
				if (_inputState[i] != buf[i]) {
					changedIndices[changedLen] = i;
					changedOldState[changedLen++] = _inputState[i];

					_inputState[i] = buf[i];
				}
			}

			for (size_t i = 0; i < 2; ++i) {
				if (!touchStateValues[i].isEqualCurrent(_touchStateValues[i])) {
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

		for (size_t i = 0; i < 2; ++i) _dispatchStick(oldSticks[i], curBuf16[i], GamepadKeyCode::L_STICK + i);

		for (uint8_t i = 0; i < changedLen; ++i) {
			auto oldState = changedOldState[i];
			auto newState = buf[changedIndices[i]];

			switch ((InputBufferOffset)changedIndices[i]) {
			case InputBufferOffset::D_PAD:
			{
				_dispatchDPad(oldState, newState);
				_dispatchButton(oldState, newState, InputMask::SQUARE, GamepadKeyCode::SQUARE);
				_dispatchButton(oldState, newState, InputMask::CROSS, GamepadKeyCode::CROSS);
				_dispatchButton(oldState, newState, InputMask::CIRCLE, GamepadKeyCode::CIRCLE);
				_dispatchButton(oldState, newState, InputMask::TRIANGLE, GamepadKeyCode::TRIANGLE);

				break;
			}
			case InputBufferOffset::L1:
			{
				_dispatchButton(oldState, newState, InputMask::L1, GamepadKeyCode::L1);
				_dispatchButton(oldState, newState, InputMask::R1, GamepadKeyCode::R1);
				_dispatchButton(oldState, newState, InputMask::SHARE, GamepadKeyCode::SHARE);
				_dispatchButton(oldState, newState, InputMask::OPTIONS, GamepadKeyCode::OPTIONS);
				_dispatchButton(oldState, newState, InputMask::L3, GamepadKeyCode::L3);
				_dispatchButton(oldState, newState, InputMask::R3, GamepadKeyCode::R3);

				break;
			}
			case InputBufferOffset::PS:
				_dispatchButton(oldState, newState, InputMask::TOUTCH_PAD_CLICK, GamepadKeyCode::TOUCH_PAD);
				break;
			case InputBufferOffset::L_TRIGGER:
				_dispatchTrigger(oldState, newState, GamepadKeyCode::L2);
				break;
			case InputBufferOffset::R_TRIGGER:
				_dispatchTrigger(oldState, newState, GamepadKeyCode::R2);
				break;
			default:
				break;
			}
		}

		if (changedTouchCount) {
			DeviceTouchStateValue touches[2];
			DeviceState::CountType n = 0;
			for (size_t i = 0; i < 2; ++i) {
				if (changedTouchStates[i]) _getTouch(touchStateValues[i], time, touches[n++]);
			}
			DeviceState k = { 0, changedTouchCount, touches };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::TOUCH, &k);
		}
	}

	bool GamepadDS4::_doOutput() {
		if (_outputDirty.exchange(false)) {
			std::shared_lock lock(_outputBufferMutex);

			memcpy(_outputState + _outputOffset, _outputBuffer, sizeof(OutputBuffer));

			return true;
		}

		return false;
	}

	DeviceState::CountType GamepadDS4::_getButton(uint8_t state, InputMask mask, DeviceStateValue* data) {
		auto val = state & (uint8_t)mask;
		data[0] = val ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		return 1;
	}

	DeviceState::CountType GamepadDS4::_getTrigger(uint8_t state, GamepadKeyCode key, DeviceStateValue* data) const {
		auto value = _translateTrigger(state);
		auto dz = _getDeadZone(key);
		data[0] = _translateDeadZone01(value, dz, value <= dz);
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

	DeviceState::CountType GamepadDS4::_getStick(uint8_t xstate, uint8_t ystate, GamepadKeyCode key, DeviceStateValue* data, DeviceState::CountType count) const {
		DeviceState::CountType c = 1;

		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;

		auto dx = _translateStick(xstate);
		auto dy = _translateStick(ystate);

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

	void GamepadDS4::_dispatchButton(uint8_t oldState, uint8_t newState, InputMask mask, GamepadKeyCode key) {
		auto oldVal = oldState & (uint8_t)mask;
		auto newVal = newState & (uint8_t)mask;
		if (oldVal != newVal) {
			auto value = newVal ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher->dispatchEvent(this, newVal ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
		}
	}

	void GamepadDS4::_dispatchTrigger(uint8_t oldState, uint8_t newState, GamepadKeyCode key) {
		auto value = _translateTrigger(newState);
		auto dz = _getDeadZone(key);
		auto oriDz = _translateTrigger(oldState) <= dz;
		auto curDz = value <= dz;
		if (!curDz || oriDz != curDz) {
			value = _translateDeadZone01(value, dz, curDz);
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher->dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void GamepadDS4::_dispatchDPad(uint8_t oldState, uint8_t newState) {
		auto oldVal = oldState & 0xF;
		auto newVal = newState & 0xF;
		if (oldVal != newVal) {
			if (newVal <= 7) {
				auto value = newVal * Math::PI_4<DeviceStateValue>;
				DeviceState k = { (DeviceState::CodeType)GamepadKeyCode::DPAD, 1, &value };
				_eventDispatcher->dispatchEvent(this, DeviceEvent::DOWN, &k);
			} else if (oldVal <= 7) {
				auto value = Math::NEGATIVE_ONE<DeviceStateValue>;
				DeviceState k = { (DeviceState::CodeType)GamepadKeyCode::DPAD, 1, &value };
				_eventDispatcher->dispatchEvent(this, DeviceEvent::UP, &k);
			}
		}
	}

	void GamepadDS4::_dispatchStick(uint16_t oldState, uint16_t newState, GamepadKeyCode key) {
		if (oldState != newState) {
			DeviceStateValue value[] = { _translateStick(newState & 0xFF) , _translateStick(newState >> 8 & 0xFF) };
			auto dz = _getDeadZone(key);
			auto dz2 = dz * dz;
			auto x = _translateStick(oldState & 0xFF), y = _translateStick(oldState >> 8 & 0xFF);
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
	}

	void GamepadDS4::_translateTouch(uint8_t* data, InternalDeviceTouchStateValue* states, size_t time) {
		if (data[0]) {
			size_t offset = 2;

			for (size_t i = 0; i < 2; ++i) {
				auto& state = states[i];

				auto isTouch = (data[offset] >> 7 & 0b1) == 0;
				auto id = data[offset] & 0x7F;
				auto x = (((data[offset + 2] & 0xF) << 8) | (data[offset + 1])) * Math::RECIPROCAL<TOUCH_PAD_RESOLUTION_MAX_X>;
				auto y = ((data[offset + 3] << 4) | (data[offset + 2] >> 4 & 0xF)) * Math::RECIPROCAL<TOUCH_PAD_RESOLUTION_MAX_Y>;

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