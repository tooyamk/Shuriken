#include "GamepadDS4.h"
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
	}

	DeviceState::CountType GamepadDS4::getState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) const {
		switch (type) {
		case DeviceStateType::KEY:
		{
			if (data && count) {
				std::shared_lock lock(_inputStateMutex);

				switch ((GamepadKeyCode)code) {
				case GamepadKeyCode::DPAD:
					return _getDPad(_inputState[(int32_t)InputBufferOffset::D_PAD], data);
				case GamepadKeyCode::SQUARE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::SQUARE], InputMask::SQUARE, data);
				case GamepadKeyCode::CROSS:
					return _getButton(_inputState[(int32_t)InputBufferOffset::CROSS], InputMask::CROSS, data);
				case GamepadKeyCode::CIRCLE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::CIRCLE], InputMask::CIRCLE, data);
				case GamepadKeyCode::TRIANGLE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::TRIANGLE], InputMask::TRIANGLE, data);
				case GamepadKeyCode::L1:
					return _getButton(_inputState[(int32_t)InputBufferOffset::L1], InputMask::L1, data);
				case GamepadKeyCode::R1:
					return _getButton(_inputState[(int32_t)InputBufferOffset::R1], InputMask::R1, data);
				case GamepadKeyCode::SHARE:
					return _getButton(_inputState[(int32_t)InputBufferOffset::SHARE], InputMask::SHARE, data);
				case GamepadKeyCode::OPTIONS:
					return _getButton(_inputState[(int32_t)InputBufferOffset::OPTIONS], InputMask::OPTIONS, data);
				case GamepadKeyCode::L3:
					return _getButton(_inputState[(int32_t)InputBufferOffset::L3], InputMask::L3, data);
				case GamepadKeyCode::R3:
					return _getButton(_inputState[(int32_t)InputBufferOffset::R3], InputMask::R3, data);
				case GamepadKeyCode::TOUCH_PAD:
					return _getButton(_inputState[(int32_t)InputBufferOffset::TOUTCH_PAD], InputMask::TOUTCH_PAD_CLICK, data);
				case GamepadKeyCode::L2:
					return _getTrigger(_inputState[(int32_t)InputBufferOffset::L2], GamepadKeyCode::L2, data);
				case GamepadKeyCode::R2:
					return _getTrigger(_inputState[(int32_t)InputBufferOffset::R2], GamepadKeyCode::R2, data);
				case GamepadKeyCode::L_STICK:
					return _getStick(_inputState[(int32_t)InputBufferOffset::LX], _inputState[(int32_t)InputBufferOffset::LY], GamepadKeyCode::L_STICK, data, count);
				case GamepadKeyCode::R_STICK:
					return _getStick(_inputState[(int32_t)InputBufferOffset::RX], _inputState[(int32_t)InputBufferOffset::RY], GamepadKeyCode::R_STICK, data, count);
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

	DeviceState::CountType GamepadDS4::setState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) {
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
				DeviceState::CodeType c = 1;
				DeviceState::ValueType l = data[0], r;
				if (count >= 2) {
					r = data[1];
					++c;
				}

				_setVibration(l, r);
				return c;
			}

			return 0;
		}
		case DeviceStateType::LED:
		{
			if (data && count) {
				DeviceState::CodeType c = 1;
				DeviceState::ValueType r = data[0], g, b;
				if (count >= 2) {
					g = data[1];
					++c;
					if (count >= 3) {
						b = data[2];
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

		/*
		{
			auto touches = buf[32];
			auto touch1 = buf[34] >> 7;
			auto touch1_id = buf[34] & 0x7F;
			auto touch2 = buf[38] >> 7;
			auto touch2_id = buf[38] & 0x7F;
			auto middle = 1920 * 2 / 5;
			auto touch_pos = (buf[34 + 1] + (buf[34 + 2] & 0xF) * 255);

			int a = 1;
		}
		*/
		
		if (inputBufferSize > sizeof(InputStateBuffer)) inputBufferSize = sizeof(InputStateBuffer);

		if (!dispatchEvent) {
			std::scoped_lock lock(_inputStateMutex);

			memcpy(_inputState, buf, inputBufferSize);

			return;
		}

		uint16_t oldSticks[2];
		InputStateBuffer changedIndices;
		InputStateBuffer changedOldState;
		uint8_t len = 0;

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
					changedIndices[len] = i;
					changedOldState[len++] = _inputState[i];

					_inputState[i] = buf[i];
				}
			}
		}

		for (size_t i = 0; i < 2; ++i) _dispatchStick(oldSticks[i], curBuf16[i], GamepadKeyCode::L_STICK + i);

		for (uint8_t i = 0; i < len; ++i) {
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
	}

	bool GamepadDS4::_doOutput() {
		if (_outputDirty.exchange(false)) {
			std::shared_lock lock(_outputBufferMutex);

			memcpy(_outputState + _outputOffset, _outputBuffer, sizeof(OutputBuffer));

			return true;
		}

		return false;
	}

	DeviceState::CountType GamepadDS4::_getButton(uint8_t state, InputMask mask, DeviceState::ValueType* data) const {
		auto val = state & (uint8_t)mask;
		data[0] = val ? Math::ONE<DeviceState::ValueType> : Math::ZERO<DeviceState::ValueType>;
		return 1;
	}

	DeviceState::CountType GamepadDS4::_getTrigger(uint8_t state, GamepadKeyCode key, DeviceState::ValueType* data) const {
		auto value = _translateTrigger(state);
		auto dz = _getDeadZone(key);
		data[0] = _translateDeadZone01(value, dz, value <= dz);
		return 1;
	}

	DeviceState::CountType GamepadDS4::_getDPad(uint8_t state, DeviceState::ValueType* data) const {
		if (auto val = state & 0xF; val <= 7) {
			data[0] = val * Math::PI_4<DeviceState::ValueType>;
		} else {
			data[0] = Math::NEGATIVE_ONE<DeviceState::ValueType>;
		}
		return 1;
	}

	DeviceState::CountType GamepadDS4::_getStick(uint8_t xstate, uint8_t ystate, GamepadKeyCode key, DeviceState::ValueType* data, DeviceState::CountType count) const {
		DeviceState::CountType c = 1;

		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;

		auto dx = _translateStick(xstate);
		auto dy = _translateStick(ystate);

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

	void GamepadDS4::_setVibration(DeviceState::ValueType left, DeviceState::ValueType right) {
		auto l = (uint8_t)(Math::clamp(left, Math::ZERO<DeviceState::ValueType>, Math::ONE<DeviceState::ValueType>) * NUMBER_255<DeviceState::ValueType>);
		auto r = (uint8_t)(Math::clamp(right, Math::ZERO<DeviceState::ValueType>, Math::ONE<DeviceState::ValueType>) * NUMBER_255<DeviceState::ValueType>);

		std::scoped_lock lock(_outputBufferMutex);

		_outputBuffer[0] = l;
		_outputBuffer[1] = r;

		_outputDirty = true;
	}

	void GamepadDS4::_setLed(DeviceState::ValueType red, DeviceState::ValueType green, DeviceState::ValueType blue) {
		auto r = (uint8_t)(Math::clamp(red, Math::ZERO<DeviceState::ValueType>, Math::ONE<DeviceState::ValueType>) * NUMBER_255<DeviceState::ValueType>);
		auto g = (uint8_t)(Math::clamp(green, Math::ZERO<DeviceState::ValueType>, Math::ONE<DeviceState::ValueType>) * NUMBER_255<DeviceState::ValueType>);
		auto b = (uint8_t)(Math::clamp(blue, Math::ZERO<DeviceState::ValueType>, Math::ONE<DeviceState::ValueType>) * NUMBER_255<DeviceState::ValueType>);

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
			auto value = newVal ? Math::ONE<DeviceState::ValueType> : Math::ZERO<DeviceState::ValueType>;
			DeviceState k = { (DeviceState::CodeType)key, 1, &value };
			_eventDispatcher.dispatchEvent(this, newVal ? DeviceEvent::DOWN : DeviceEvent::UP, &k);
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
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void GamepadDS4::_dispatchDPad(uint8_t oldState, uint8_t newState) {
		auto oldVal = oldState & 0xF;
		auto newVal = newState & 0xF;
		if (oldVal != newVal) {
			if (newVal <= 7) {
				auto value = newVal * Math::PI_4<DeviceState::ValueType>;
				DeviceState k = { (DeviceState::CodeType)GamepadKeyCode::DPAD, 1, &value };
				_eventDispatcher.dispatchEvent(this, DeviceEvent::DOWN, &k);
			} else if (oldVal <= 7) {
				auto value = Math::NEGATIVE_ONE<DeviceState::ValueType>;
				DeviceState k = { (DeviceState::CodeType)GamepadKeyCode::DPAD, 1, &value };
				_eventDispatcher.dispatchEvent(this, DeviceEvent::UP, &k);
			}
		}
	}

	void GamepadDS4::_dispatchStick(uint16_t oldState, uint16_t newState, GamepadKeyCode key) {
		if (oldState != newState) {
			DeviceState::ValueType value[] = { _translateStick(newState & 0xFF) , _translateStick(newState >> 8 & 0xFF) };
			auto dz = _getDeadZone(key);
			auto dz2 = dz * dz;
			auto x = _translateStick(oldState & 0xFF), y = _translateStick(oldState >> 8 & 0xFF);
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
	}
}