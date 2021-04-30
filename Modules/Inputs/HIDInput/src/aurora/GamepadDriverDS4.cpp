#include "GamepadDriverDS4.h"
#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	GamepadDriverDS4::GamepadDriverDS4(Input& input, extensions::HIDDevice& hid) : GamepadDriverBase(input, hid) {
	}

	GamepadDriverDS4::~GamepadDriverDS4() {
	}

	size_t GamepadDriverDS4::getInputLength() const {
		return INPUT_BUFFER_LENGTH;
	}

	size_t GamepadDriverDS4::getOutputLength() const {
		return OUTPUT_BUFFER_LENGTH;
	}

	bool GamepadDriverDS4::init(void* inputState, void* outputState) {
		using namespace aurora::extensions;

		auto isBluetooth = false;
		uint8_t buf[65];
		do {
			if (auto rst = HID::read(*_hid, buf, sizeof(buf), HID::IN_TIMEOUT_BLOCKING); HID::isSuccess(rst)) {
				isBluetooth = rst > 64;
				break;
			} else if (rst == HID::OUT_ERROR) {
				return false;
			}
		} while (true);

		auto op = (uint8_t*)outputState;

		memset(outputState, 0, OUTPUT_BUFFER_LENGTH);
		if (isBluetooth) {
			op[0] = 0x11;
			op[1] = 0x80;
			op[3] = 0xFF;

			_inputOffset = 3;
			_outputOffset = 6;
		} else {
			op[0] = 0x05;
			op[1] = 0xFF;

			_inputOffset = 1;
			_outputOffset = 4;
		}

		memcpy(inputState, buf, INPUT_BUFFER_LENGTH);

		return true;
	}

	bool GamepadDriverDS4::readStateFromDevice(void* inputState) const {
		using namespace aurora::extensions;

		return HID::isSuccess(HID::read(*_hid, inputState, sizeof(INPUT_BUFFER_LENGTH), 0));
	}

	float32_t GamepadDriverDS4::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace aurora::enum_operators;

		auto data = (const uint8_t*)inputState;

		float32_t val;

		if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= MAX_AXIS_KEY) {
			switch (cf.code) {
			case GamepadKeyCode::AXIS_1:
			case GamepadKeyCode::AXIS_1 + 1:
			case GamepadKeyCode::AXIS_1 + 2:
				val = data[_inputOffset + (size_t)(cf.code - GamepadKeyCode::AXIS_1)] * Math::RECIPROCAL<float32_t((std::numeric_limits<uint8_t>::max)())>;
				break;
			case GamepadKeyCode::AXIS_1 + 3:
			case GamepadKeyCode::AXIS_1 + 4:
				val = data[_inputOffset + (size_t)InputOffset::L_TRIGGER + (size_t)(cf.code - GamepadKeyCode::AXIS_1 - 3)] * Math::RECIPROCAL<float32_t((std::numeric_limits<uint8_t>::max)())>;
				break;
			case GamepadKeyCode::AXIS_1 + 5:
				val = data[_inputOffset + (size_t)InputOffset::RY] * Math::RECIPROCAL<float32_t((std::numeric_limits<uint8_t>::max)())>;
				break;
			default:
				val = defaultVal;
				break;
			}
		} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= MAX_BUTTON_KEY) {
			auto i = (size_t)(cf.code - GamepadKeyCode::BUTTON_1);
			val = data[_inputOffset + BUTTON_OFFSET[i]] & BUTTON_MASK[i] ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		} else {
			val = defaultVal;
		}

		return translate(val, cf.flags);
	}

	float32_t GamepadDriverDS4::readDpadDataFromInputState(const void* inputState) const {
		if (auto val = ((const uint8_t*)inputState)[_inputOffset + (int32_t)InputOffset::D_PAD] & 0xF; val <= 7) return val * Math::PI_4<DeviceStateValue>;

		return Math::NEGATIVE_ONE<DeviceStateValue>;
	}

	DeviceState::CountType GamepadDriverDS4::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputState, void* custom, ReadStateStartCallback readStateStartCallback, ReadStateEndCallback readStateEndCallback) const {
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
			//todo
			return 0;
		}

		return 0;
	}

	void GamepadDriverDS4::customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const {
		DeviceTouchStateValue touches[4];
		size_t offset = _inputOffset + (size_t)InputOffset::FINGER1;
		_parseTouches((const uint8_t*)oldInputState + offset, touches);
		_parseTouches((const uint8_t*)newInputState + offset, touches + 2);
	}

	bool GamepadDriverDS4::writeStateToDevice(const void* outputState) const {
		using namespace aurora::extensions;

		return HID::isSuccess(HID::write(*_hid, outputState, OUTPUT_BUFFER_LENGTH, 0));
	}

	DeviceState::CountType GamepadDriverDS4::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* custom, WriteToOutputStateCallback writeToOutputStateCallback) const {
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

				writeToOutputStateCallback(data, sizeof(data), _outputOffset, custom);

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

				writeToOutputStateCallback(data, sizeof(data), _outputOffset + 2, custom);

				return c;
			}
		}

		return 0;
	}

	void GamepadDriverDS4::setKeyMapping(GamepadKeyMapping& dst, const GamepadKeyMapping* src) const {
		using namespace aurora::enum_operators;

		if (src) {
			dst = *src;
		} else {
			dst.clear();

			dst.set(GamepadVirtualKeyCode::L_STICK_X, GamepadKeyCode::AXIS_1);
			dst.set(GamepadVirtualKeyCode::L_STICK_Y, GamepadKeyCode::AXIS_1 + 1);
			dst.set(GamepadVirtualKeyCode::R_STICK_X, GamepadKeyCode::AXIS_1 + 2);
			dst.set(GamepadVirtualKeyCode::R_STICK_Y, GamepadKeyCode::AXIS_1 + 5);
			dst.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 3);
			dst.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 4);

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

		dst.undefinedCompletion(MAX_AXES, MAX_BUTTONS);
	}

	void GamepadDriverDS4::_parseTouches(const uint8_t* data, DeviceTouchStateValue* states) {
		size_t offset = 0;
		for (size_t i = 0; i < 2; ++i) {
			auto& state = states[i];

			auto isTouch = (data[offset] >> 7 & 0b1) == 0;
			auto id = data[offset] & 0x7F;
			auto x = (((data[offset + 2] & 0xF) << 8) | (data[offset + 1])) * Math::RECIPROCAL<TOUCH_PAD_MAX_X>;
			auto y = ((data[offset + 3] << 4) | (data[offset + 2] >> 4 & 0xF)) * Math::RECIPROCAL<TOUCH_PAD_MAX_Y>;

			state.code = 0;
			state.fingerID = id;
			state.phase = isTouch ? DeviceTouchPhase::MOVE : DeviceTouchPhase::END;
			state.position.set(x, y);

			offset += 4;
		}
	}
}