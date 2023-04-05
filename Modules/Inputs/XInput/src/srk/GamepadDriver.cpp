#include "GamepadDriver.h"
#include "Input.h"

namespace srk::modules::inputs::xinput {
	GamepadDriver::GamepadDriver(Input& input, DWORD index) :
		_input(input),
		_index(index) {
	}

	GamepadDriver::~GamepadDriver() {
		close();
	}

	size_t GamepadDriver::getInputBufferLength() const {
		return sizeof(XINPUT_STATE) + HEADER_LENGTH;
	}

	size_t GamepadDriver::getOutputBufferLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputBuffer, void* outputBuffer) {
		((uint8_t*)inputBuffer)[0] = 0;

		return true;
	}

	bool GamepadDriver::isBufferReady(const void* buffer) const {
		return ((const uint8_t*)buffer)[0];
	}

	std::optional<bool> GamepadDriver::readFromDevice(void* inputBuffer) const {
		auto raw = (uint8_t*)inputBuffer;
		if (XInputGetState(_index, (XINPUT_STATE*)(raw + HEADER_LENGTH)) == ERROR_SUCCESS) {
			raw[0] = 1;
			
			return std::make_optional(true);
		}

		return std::nullopt;
	}

	float32_t GamepadDriver::readFromInputBuffer(const void* inputBuffer, GamepadKeyCode keyCode) const {
		using namespace srk::enum_operators;

		if (!isBufferReady(inputBuffer)) return -1.0f;

		auto data = (const XINPUT_STATE*)((const uint8_t*)inputBuffer + HEADER_LENGTH);

		if (keyCode >= GamepadKeyCode::AXIS_1 && keyCode <= MAX_AXIS_KEY_CODE) {
			switch (keyCode) {
			case GamepadKeyCode::AXIS_1:
				return _normalizeThumb(data->Gamepad.sThumbLX);
			case GamepadKeyCode::AXIS_1 + 1:
				return _normalizeThumb(data->Gamepad.sThumbLY);
			case GamepadKeyCode::AXIS_1 + 2:
				return _normalizeTrigger(data->Gamepad.bLeftTrigger);
			case GamepadKeyCode::AXIS_1 + 3:
				return _normalizeThumb(data->Gamepad.sThumbRX);
			case GamepadKeyCode::AXIS_1 + 4:
				return _normalizeThumb(data->Gamepad.sThumbRY);
			case GamepadKeyCode::AXIS_1 + 5:
				return _normalizeTrigger(data->Gamepad.bRightTrigger);
			case GamepadKeyCode::HAT_1:
			{
				switch (data->Gamepad.wButtons & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT)) {
				case XINPUT_GAMEPAD_DPAD_UP:
					return 0.0f;
				case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT:
					return Math::ONE_EIGHTH<float32_t>;
				case XINPUT_GAMEPAD_DPAD_RIGHT:
					return Math::ONE_QUARTER<float32_t>;
				case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT:
					return 3.0f * Math::ONE_EIGHTH<float32_t>;
				case XINPUT_GAMEPAD_DPAD_DOWN:
					return Math::ONE_HALF<float32_t>;
				case XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT:
					return 5.0f * Math::ONE_EIGHTH<float32_t>;
				case XINPUT_GAMEPAD_DPAD_LEFT:
					return 3.0f * Math::ONE_QUARTER<float32_t>;
				case XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT:
					return 7.0f * Math::ONE_EIGHTH<float32_t>;
				default:
					return -1.0f;
				}
			}
			default:
				return -1.0f;
			}
		} else if (keyCode >= GamepadKeyCode::BUTTON_1 && keyCode <= MAX_BUTTON_KEY_CODE) {
			return data->Gamepad.wButtons & BUTTON_MASK[(size_t)(keyCode - GamepadKeyCode::BUTTON_1)] ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		} else {
			return -1.0f;
		}
	}

	DeviceState::CountType GamepadDriver::customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
		const void* inputBuffer, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateEndCallback readStateEndCallback) const {
		return 0;
	}

	void GamepadDriver::customDispatch(const void* oldInputBuffer, const void* newInputBuffer, void* custom, DispatchCallback dispatchCallback) const {
	}

	bool GamepadDriver::writeToDevice(const void* outputBuffer) const {
		return true;
	}

	DeviceState::CountType GamepadDriver::customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputBuffer, void* custom,
		ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateEndCallback writeStateEndCallback) const {
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

			dst.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 2);
			dst.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 5);

			dst.set(GamepadVirtualKeyCode::DPAD_LEFT, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_X | GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_RIGHT, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_X | GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::DPAD_DOWN, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_Y | GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_UP, GamepadKeyCode::HAT_1, GamepadKeyFlag::AXIS_Y | GamepadKeyFlag::HALF_BIG);

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

		dst.undefinedCompletion<GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END, GamepadVirtualKeyCode::UNDEFINED_AXIS_1>(MAX_AXES);
		dst.undefinedCompletion<GamepadKeyCode::HAT_1, GamepadKeyCode::HAT_END, GamepadVirtualKeyCode::UNDEFINED_HAT_1>(MAX_HATS);
		dst.undefinedCompletion<GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END, GamepadVirtualKeyCode::UNDEFINED_BUTTON_1>(MAX_BUTTONS);
	}

	void GamepadDriver::close() {
	}

	void GamepadDriver::_setVibration(DeviceStateValue left, DeviceStateValue right) const {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = Math::clamp(left, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * (std::numeric_limits<uint16_t>::max)();
		vibration.wRightMotorSpeed = Math::clamp(right, Math::ZERO<DeviceStateValue>, Math::ONE<DeviceStateValue>) * (std::numeric_limits<uint16_t>::max)();
		XInputSetState(_index, &vibration);
	}
}