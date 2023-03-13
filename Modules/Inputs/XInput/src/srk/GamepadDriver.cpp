#include "GamepadDriver.h"
#include "Input.h"

namespace srk::modules::inputs::xinput {
	GamepadDriver::GamepadDriver(Input& input, const DeviceInfo& info) :
		_input(input),
		_index(((InternalGUID&)*info.guid.getData()).index - 1) {
	}

	GamepadDriver::~GamepadDriver() {
	}

	size_t GamepadDriver::getInputLength() const {
		return sizeof(XINPUT_STATE) + HEADER_LENGTH;
	}

	size_t GamepadDriver::getOutputLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputState, void* outputState) {
		((uint8_t*)inputState)[0] = 0;

		return true;
	}

	bool GamepadDriver::isStateReady(const void* state) const {
		return ((const uint8_t*)state)[0];
	}

	bool GamepadDriver::readStateFromDevice(void* inputState) const {
		auto raw = (uint8_t*)inputState;
		if (XInputGetState(_index, (XINPUT_STATE*)(raw + HEADER_LENGTH)) == ERROR_SUCCESS) {
			raw[0] = 1;
			
			return true;
		}

		return false;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace srk::enum_operators;

		constexpr auto diagonal = 0.5f / Math::SQRT2<float32_t>;

		float32_t val;
		if (auto raw = (const uint8_t*)inputState; raw[0]) {
			auto data = (const XINPUT_STATE*)(raw + HEADER_LENGTH);

			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= MAX_AXIS_KEY_CODE) {
				switch (cf.code) {
				case GamepadKeyCode::AXIS_1:
					val = _normalizeThumb(data->Gamepad.sThumbLX);
					break;
				case GamepadKeyCode::AXIS_1 + 1:
					val = _normalizeThumb(data->Gamepad.sThumbLY);
					break;
				case GamepadKeyCode::AXIS_1 + 2:
					val = _normalizeTrigger(data->Gamepad.bLeftTrigger);
					break;
				case GamepadKeyCode::AXIS_1 + 3:
					val = _normalizeThumb(data->Gamepad.sThumbRX);
					break;
				case GamepadKeyCode::AXIS_1 + 4:
					val = _normalizeThumb(data->Gamepad.sThumbRY);
					break;
				case GamepadKeyCode::AXIS_1 + 5:
					val = _normalizeTrigger(data->Gamepad.bRightTrigger);
					break;
				case GamepadKeyCode::AXIS_1 + 6:
				{
					switch (data->Gamepad.wButtons & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT)) {
					case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT:
					case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
						val = 0.5f + diagonal;
						break;
					case XINPUT_GAMEPAD_DPAD_RIGHT:
						val = 1.f;
						break;
					case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
					case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT:
						val = 0.5f - diagonal;
						break;
					case XINPUT_GAMEPAD_DPAD_LEFT:
						val = 0.f;
						break;
					default:
						val = 0.5f;
						break;
					}

					break;
				}
				case GamepadKeyCode::AXIS_1 + 7:
				{
					switch (data->Gamepad.wButtons & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT)) {
					case XINPUT_GAMEPAD_DPAD_UP:
						val = 1.f;
						break;
					case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT:
					case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT:
						val = 0.5f + diagonal;
						break;
					case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
					case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
						val = 0.5f - diagonal;
						break;
					case XINPUT_GAMEPAD_DPAD_DOWN:
						val = 0.f;
						break;
					default:
						val = 0.5f;
						break;
					}

					break;
				}
				default:
					val = defaultVal;
					break;
				}
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= MAX_BUTTON_KEY_CODE) {
				val = data->Gamepad.wButtons & BUTTON_MASK[(size_t)(cf.code - GamepadKeyCode::BUTTON_1)] ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
			} else {
				val = defaultVal;
			}
		} else {
			val = defaultVal;
		}

		return translate(val, cf.flags);
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const {
	}

	bool GamepadDriver::writeStateToDevice(const void* outputState) const {
		return false;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const {
		if (type == DeviceStateType::VIBRATION) {
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

		return 0;
	}

	void GamepadDriver::setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const {
		using namespace srk::enum_operators;

		if (src) {
			dst = *src;
		} else {
			dst.clear();

			dst.set(GamepadVirtualKeyCode::L_STICK_X_LEFT, GamepadKeyCode::AXIS_1, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::L_STICK_X_RIGHT, GamepadKeyCode::AXIS_1, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::L_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::L_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_BIG);

			dst.set(GamepadVirtualKeyCode::R_STICK_X_LEFT, GamepadKeyCode::AXIS_1 + 3, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::R_STICK_X_RIGHT, GamepadKeyCode::AXIS_1 + 3, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::R_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 4, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::R_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 4, GamepadKeyFlag::HALF_BIG);

			dst.set(GamepadVirtualKeyCode::DPAD_LEFT, GamepadKeyCode::AXIS_1 + 6, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_RIGHT, GamepadKeyCode::AXIS_1 + 6, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::DPAD_DOWN, GamepadKeyCode::AXIS_1 + 7, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_UP, GamepadKeyCode::AXIS_1 + 7, GamepadKeyFlag::HALF_BIG);

			dst.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 2);
			dst.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 5);

			dst.set(GamepadVirtualKeyCode::A, GamepadKeyCode::BUTTON_1);
			dst.set(GamepadVirtualKeyCode::B, GamepadKeyCode::BUTTON_1 + 1);
			dst.set(GamepadVirtualKeyCode::X, GamepadKeyCode::BUTTON_1 + 2);
			dst.set(GamepadVirtualKeyCode::Y, GamepadKeyCode::BUTTON_1 + 3);
			dst.set(GamepadVirtualKeyCode::L_SHOULDER, GamepadKeyCode::BUTTON_1 + 4);
			dst.set(GamepadVirtualKeyCode::R_SHOULDER, GamepadKeyCode::BUTTON_1 + 5);
			dst.set(GamepadVirtualKeyCode::SELECT, GamepadKeyCode::BUTTON_1 + 6);
			dst.set(GamepadVirtualKeyCode::START, GamepadKeyCode::BUTTON_1 + 7);
			dst.set(GamepadVirtualKeyCode::L_THUMB, GamepadKeyCode::BUTTON_1 + 8);
			dst.set(GamepadVirtualKeyCode::R_THUMB, GamepadKeyCode::BUTTON_1 + 9);
		}

		dst.undefinedCompletion(MAX_AXES, MAX_BUTTONS);
	}

	void GamepadDriver::_setVibration(DeviceStateValue left, DeviceStateValue right) const {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = Math::clamp(left, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * (std::numeric_limits<uint16_t>::max)();
		vibration.wRightMotorSpeed = Math::clamp(right, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * (std::numeric_limits<uint16_t>::max)();
		XInputSetState(_index, &vibration);
	}
}