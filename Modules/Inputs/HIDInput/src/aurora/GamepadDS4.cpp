#include "GamepadDS4.h"
#include "aurora/Debug.h"

namespace aurora::modules::inputs::hid_input {
	GamepadDS4::GamepadDS4(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid) : GamepadBase(input, info, hid) {
		_state[(std::underlying_type_t<InputBufferOffset>)InputBufferOffset::D_PAD] = 0x8;
	}

	Key::CountType GamepadDS4::getKeyState(Key::CodeType keyCode, Key::ValueType* data, Key::CountType count) const {
		return 0;
	}

	void GamepadDS4::_parse(bool dispatchEvent, ReadBuffer& readBuffer, size_t readBufferSize) {
		using namespace aurora::enum_operators;

		auto buf = readBuffer + 1;
		--readBufferSize;
		if (readBufferSize > sizeof(StateBuffer)) readBufferSize = sizeof(StateBuffer);

		if (!dispatchEvent) {
			std::scoped_lock lock(_mutex);

			memcpy(_state, buf, readBufferSize);

			return;
		}

		uint16_t oldSticks[2];
		StateBuffer changedIndices;
		StateBuffer changedOldState;
		uint8_t len = 0;

		auto oldBuf16 = (uint16_t*)_state;
		auto curBuf16 = (uint16_t*)buf;

		{
			std::scoped_lock lock(_mutex);

			for (size_t i = 0; i < 2; ++i) {
				oldSticks[i] = oldBuf16[i];
				oldBuf16[i] = curBuf16[i];
			}

			for (uint8_t i = sizeof(oldSticks); i < readBufferSize; ++i) {
				if (_state[i] != buf[i]) {
					changedIndices[len] = i;
					changedOldState[len++] = _state[i];

					_state[i] = buf[i];
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

	void GamepadDS4::_dispatchButton(uint8_t oldState, uint8_t newState, InputMask mask, GamepadKeyCode key) {
		auto oldVal = oldState & (uint8_t)mask;
		auto newVal = newState & (uint8_t)mask;
		if (oldVal != newVal) {
			auto value = newVal ? Math::ONE<Key::ValueType> : Math::ZERO<Key::ValueType>;
			Key k = { (Key::CodeType)key, 1, &value };
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
			Key k = { (Key::CodeType)key, 1, &value };
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
		}
	}

	void GamepadDS4::_dispatchDPad(uint8_t oldState, uint8_t newState) {
		auto oldVal = oldState & 0xF;
		auto newVal = newState & 0xF;
		if (oldVal != newVal) {
			if (newVal <= 7) {
				auto value = newVal * Math::PI_4<Key::ValueType>;
				Key k = { (Key::CodeType)GamepadKeyCode::DPAD, 1, &value };
				_eventDispatcher.dispatchEvent(this, DeviceEvent::DOWN, &k);
			} else if (oldVal <= 7) {
				auto value = Math::NEGATIVE_ONE<Key::ValueType>;
				Key k = { (Key::CodeType)GamepadKeyCode::DPAD, 1, &value };
				_eventDispatcher.dispatchEvent(this, DeviceEvent::UP, &k);
			}
		}
	}

	void GamepadDS4::_dispatchStick(uint16_t oldState, uint16_t newState, GamepadKeyCode key) {
		if (oldState != newState) {
			Key::ValueType value[] = { _translateStick(newState & 0xFF) , _translateStick(newState >> 8 & 0xFF) };
			auto dz = _getDeadZone(key);
			auto dz2 = dz * dz;
			auto x = _translateStick(oldState & 0xFF), y = _translateStick(oldState >> 8 & 0xFF);
			auto oriDz = x * x + y * y <= dz2;
			auto d2 = value[0] * value[0] + value[1] * value[1];
			if (d2 > Math::ONE<Key::ValueType>) d2 = Math::ONE<Key::ValueType>;
			auto curDz = d2 <= dz2;
			if (!oriDz || oriDz != curDz) {
				if (curDz) {
					value[0] = Math::NEGATIVE_ONE<Key::ValueType>;
					value[1] = Math::ZERO<Key::ValueType>;
				} else {
					value[0] = std::atan2(value[1], value[0]) + Math::PI_2<float32_t>;
					if (value[0] < Math::ZERO<Key::ValueType>) value[0] += Math::PI2<float32_t>;
					value[1] = _translateDeadZone01(d2 < Math::ONE<Key::ValueType> ? std::sqrt(d2) : Math::ONE<Key::ValueType>, dz, false);
				}
				Key k = { (Key::CodeType)key, 2, value };
				_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &k);
			}
		}
	}
}