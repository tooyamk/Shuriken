#include "GamepadDriverDS4.h"
#include "Input.h"

namespace srk::modules::inputs::hid_input {
	GamepadDriverDS4::GamepadDriverDS4(Input& input, extensions::HIDDevice& hid) : GamepadDriverBase(input, hid),
		_state(0) {
	}

	GamepadDriverDS4::~GamepadDriverDS4() {
	}

	size_t GamepadDriverDS4::getInputLength() const {
		return INPUT_BUFFER_LENGTH + HEADER_LENGTH;
	}

	size_t GamepadDriverDS4::getOutputLength() const {
		return OUTPUT_BUFFER_LENGTH + HEADER_LENGTH;
	}

	bool GamepadDriverDS4::init(void* inputState, void* outputState) {
		using namespace srk::extensions;

		((uint8_t*)inputState)[0] = 0;
		memset(outputState, 0, getOutputLength());

		_state = 0;

		return true;
	}

	bool GamepadDriverDS4::isStateReady(const void* state) const {
		return ((const uint8_t*)state)[0];
	}

	bool GamepadDriverDS4::readStateFromDevice(void* inputState) const {
		using namespace srk::extensions;

		auto data = (uint8_t*)inputState;
		if (auto s = _state.load(); s) {
			if (HID::isSuccess(HID::read(*_hid, data + HEADER_LENGTH, INPUT_BUFFER_LENGTH, 0))) {
				data[0] = s & 0b1111;
				return true;
			}
		} else {
			uint8_t buf[65];
			if (auto rst = HID::read(*_hid, buf, sizeof(buf), 0); HID::isSuccess(rst)) {
				uint8_t inputOffset, outputOffset;
				if (rst > 64) {
					inputOffset = 3;
					outputOffset = 6;
				} else {
					inputOffset = 1;
					outputOffset = 4;
				}

				data[0] = inputOffset;
				memcpy(data + HEADER_LENGTH, buf, INPUT_BUFFER_LENGTH);

				_state = outputOffset << 4 | inputOffset;

				return true;
			}
		}

		return false;
	}

	float32_t GamepadDriverDS4::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace srk::enum_operators;

		auto data = (const uint8_t*)inputState;

		float32_t val;
		if (auto offset = data[0]; offset) {
			data += HEADER_LENGTH + offset;

			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= MAX_AXIS_KEY_CODE) {
				switch (cf.code) {
				case GamepadKeyCode::AXIS_1:
				case GamepadKeyCode::AXIS_1 + 1:
				case GamepadKeyCode::AXIS_1 + 2:
					val = data[(size_t)(cf.code - GamepadKeyCode::AXIS_1)] / 255.0f;
					break;
				case GamepadKeyCode::AXIS_1 + 3:
				case GamepadKeyCode::AXIS_1 + 4:
					val = data[(size_t)InputOffset::L_TRIGGER + (size_t)(cf.code - GamepadKeyCode::AXIS_1 - 3)] / 255.0f;
					break;
				case GamepadKeyCode::AXIS_1 + 5:
					val = data[(size_t)InputOffset::RY] / 255.0f;
					break;
				default:
					val = defaultVal;
					break;
				}
			} else if (cf.code >= GamepadKeyCode::HAT_1 && cf.code <= MAX_HAT_KEY_CODE) {
				if (auto i = data[(size_t)InputOffset::D_PAD] & 0xF; i < 8) {
					val = i * Math::ONE_EIGHTH<float32_t>;
				} else {
					val = -1.0f;
				}
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= MAX_BUTTON_KEY_CODE) {
				auto i = (size_t)(cf.code - GamepadKeyCode::BUTTON_1);
				val = data[BUTTON_OFFSET[i]] & BUTTON_MASK[i] ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			} else {
				val = defaultVal;
			}
		} else {
			val = defaultVal;
		}

		return translate(val, cf.flags);
	}

	DeviceState::CountType GamepadDriverDS4::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const {
		if (type == DeviceStateType::TOUCH_RESOLUTION) {
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
		} else if (type == DeviceStateType::TOUCH) {
			if (values && count) {
				auto data = (const uint8_t*)inputState;
				if (auto offset = data[0]; offset) {
					data += offset + (size_t)InputOffset::FINGER1;
					DeviceTouchStateValue rawTouches[2], touches[2];

					readStateStartCallback(custom);

					_parseTouches(data, rawTouches);

					readStateEndCallback(custom);

					DeviceState::CountType c = 0;
					for (size_t i = 0; i < 2; ++i) {
						if (rawTouches[i].isTouched) ((DeviceTouchStateValue*)values)[c++] = rawTouches[i];
						if (++c >= count) break;
					}

					return c;
				}
			}

			return 0;
		}

		return 0;
	}

	void GamepadDriverDS4::customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const {
		auto oldData = (const uint8_t*)oldInputState;
		auto newData = (const uint8_t*)newInputState;

		auto oldOffset = oldData[0], newOffset = newData[0];

		if (!oldOffset || !newOffset) return;

		DeviceTouchStateValue rawTouches[4];
		_parseTouches(oldData + HEADER_LENGTH + oldOffset + (size_t)InputOffset::FINGER1, rawTouches);
		_parseTouches(newData + HEADER_LENGTH + newOffset + (size_t)InputOffset::FINGER1, rawTouches + 2);

		DeviceTouchStateValue touches[4];
		size_t count = 0;
		TouchCollection<4> tc(rawTouches);
		tc.query([&](const uint8_t* indices, size_t n) {
			auto idx1 = indices[0];
			auto& t1 = rawTouches[idx1];

			if (n == 1) {
				if (idx1 < 2) {
					if (t1.isTouched) {
						t1.isTouched = false;
						touches[count++] = t1;
					}
				} else {
					if (t1.isTouched) touches[count++] = t1;
				}
			} else {
				if (idx1 >= 2) {
					if (t1.isTouched) {
						touches[count++] = t1;
					} else {
						return;
					}
				}

				for (size_t i = 1; i < n; ++i) {
					auto idx2 = indices[i];
					auto& t2 = rawTouches[idx2];

					if (t1.isTouched == t2.isTouched) {
						if (t2.isTouched) {
							if (idx2 >= 2 && t1.position != t2.position) touches[count++] = t2;
						}
					} else {
						if (!t2.isTouched) {
							if (idx2 >= 2) touches[count++] = t2;
							return;
						}
					}

					t1 = t2;
					idx1 = idx2;
				}
				
			}
		});

		if (count) {
			DeviceState ds = { (DeviceState::CodeType)0, count, touches };
			dispatchCallback(DeviceEvent::TOUCH, &ds, custom);
		}
	}

	bool GamepadDriverDS4::writeStateToDevice(const void* outputState) const {
		using namespace srk::extensions;

		return HID::isSuccess(HID::write(*_hid, ((const uint8_t*)outputState) + HEADER_LENGTH, OUTPUT_BUFFER_LENGTH, 0));
	}

	DeviceState::CountType GamepadDriverDS4::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const {

		auto data = (uint8_t*)outputState;
		auto offset = data[0];
		if (!offset) offset = _getOutputOffset(_state.load());

		if (offset) {
			if (type == DeviceStateType::VIBRATION) {
				if (values && count) {
					DeviceState::CodeType c = 1;
					DeviceStateValue lr[2];
					lr[0] = ((DeviceStateValue*)values)[0];
					if (count >= 2) {
						lr[1] = ((DeviceStateValue*)values)[1];
						++c;
					} else {
						lr[1] = Math::ZERO<DeviceStateValue>;
					}

					constexpr auto MAX = DeviceStateValue(255);

					uint8_t data[2];
					for (size_t i = 0; i < 2; ++i) data[i] = Math::clamp(lr[i], Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * MAX;

					uint8_t expect = 1;

					writeStateStartCallback(custom);

					_writeOutputStateInit(outputState, offset);
					memcpy((uint8_t*)outputState + HEADER_LENGTH + offset, data, sizeof(data));

					writeStateEndCallback(custom);

					return c;
				}
			} else if (type == DeviceStateType::LED) {
				if (values && count) {
					DeviceState::CodeType c = 1;
					DeviceStateValue rgb[3];
					rgb[0] = ((DeviceStateValue*)values)[0];
					if (count >= 2) {
						rgb[1] = ((DeviceStateValue*)values)[1];
						++c;
						if (count >= 3) {
							rgb[2] = ((DeviceStateValue*)values)[2];
							++c;
						}
					}

					for (size_t i = c; i < 3; ++i) rgb[i] = Math::ZERO<DeviceStateValue>;

					constexpr auto MAX = DeviceStateValue(255);

					uint8_t data[3];
					for (size_t i = 0; i < 3; ++i) data[i] = Math::clamp(rgb[i], Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * MAX;

					writeStateStartCallback(custom);

					_writeOutputStateInit(outputState, offset);
					memcpy((uint8_t*)outputState + HEADER_LENGTH + offset + 2, data, sizeof(data));

					writeStateEndCallback(custom);

					return c;
				}
			}
		}

		return 0;
	}

	void GamepadDriverDS4::setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const {
		using namespace srk::enum_operators;

		if (src) {
			dst = *src;
		} else {
			dst.clear();

			dst.set(GamepadVirtualKeyCode::L_STICK_X_LEFT, GamepadKeyCode::AXIS_1, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::L_STICK_X_RIGHT, GamepadKeyCode::AXIS_1, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::L_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::L_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

			dst.set(GamepadVirtualKeyCode::R_STICK_X_LEFT, GamepadKeyCode::AXIS_1 + 2, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::R_STICK_X_RIGHT, GamepadKeyCode::AXIS_1 + 2, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::R_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 5, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::R_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 5, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

			dst.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 3);
			dst.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 4);

			dst.set(GamepadVirtualKeyCode::DPAD_LEFT, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_X | GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_RIGHT, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_X | GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::DPAD_DOWN, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_Y | GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_UP, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_Y | GamepadKeyFlag::HALF_BIG);

			dst.set(GamepadVirtualKeyCode::SQUARE, GamepadKeyCode::BUTTON_1);
			dst.set(GamepadVirtualKeyCode::CROSS, GamepadKeyCode::BUTTON_1 + 1);
			dst.set(GamepadVirtualKeyCode::CIRCLE, GamepadKeyCode::BUTTON_1 + 2);
			dst.set(GamepadVirtualKeyCode::TRIANGLE, GamepadKeyCode::BUTTON_1 + 3);
			dst.set(GamepadVirtualKeyCode::L1, GamepadKeyCode::BUTTON_1 + 4);
			dst.set(GamepadVirtualKeyCode::R1, GamepadKeyCode::BUTTON_1 + 5);
			dst.set(GamepadVirtualKeyCode::L2_BUTTON, GamepadKeyCode::BUTTON_1 + 6);
			dst.set(GamepadVirtualKeyCode::R2_BUTTON, GamepadKeyCode::BUTTON_1 + 7);
			dst.set(GamepadVirtualKeyCode::SHARE, GamepadKeyCode::BUTTON_1 + 8);
			dst.set(GamepadVirtualKeyCode::OPTIONS, GamepadKeyCode::BUTTON_1 + 9);
			dst.set(GamepadVirtualKeyCode::L3, GamepadKeyCode::BUTTON_1 + 10);
			dst.set(GamepadVirtualKeyCode::R3, GamepadKeyCode::BUTTON_1 + 11);
			dst.set(GamepadVirtualKeyCode::UNDEFINED_BUTTON_1, GamepadKeyCode::BUTTON_1 + 12);
			dst.set(GamepadVirtualKeyCode::TOUCH_PAD, GamepadKeyCode::BUTTON_1 + 13);
		}

		dst.undefinedCompletion<GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END, GamepadVirtualKeyCode::UNDEFINED_AXIS_1>(MAX_AXES);
		dst.undefinedCompletion<GamepadKeyCode::HAT_1, GamepadKeyCode::HAT_END, GamepadVirtualKeyCode::UNDEFINED_HAT_1>(MAX_HATS);
		dst.undefinedCompletion<GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END, GamepadVirtualKeyCode::UNDEFINED_BUTTON_1>(MAX_BUTTONS);
	}

	void GamepadDriverDS4::_writeOutputStateInit(void* outputState, uint8_t offset) const {
		auto raw = (uint8_t*)outputState;

		if (!raw[0]) {
			raw[0] = offset;

			if (offset == 6) {
				uint8_t data[] = { 0x11, 0x80, 0, 0xFF };
				memcpy(raw + HEADER_LENGTH, data, sizeof(data));
			} else {
				uint8_t data[] = { 0x05, 0xFF };
				memcpy(raw + HEADER_LENGTH, data, sizeof(data));
			}
		}
	}

	void GamepadDriverDS4::_parseTouches(const uint8_t* data, DeviceTouchStateValue* states) {
		size_t offset = 0;
		for (size_t i = 0; i < 2; ++i) {
			auto& state = states[i];

			state.code = 0;
			state.fingerID = data[offset] & 0x7F;
			state.isTouched = (data[offset] >> 7 & 0b1) == 0;
			state.position.set(
				(((data[offset + 2] & 0xF) << 8) | (data[offset + 1])) / TOUCH_PAD_MAX_X, 
				((data[offset + 3] << 4) | (data[offset + 2] >> 4 & 0xF)) / TOUCH_PAD_MAX_Y);

			offset += 4;
		}
	}
}