#include "GamepadDriver.h"
#include "Input.h"

#include <wbemidl.h>
#include <oleauto.h>
#include <dinput.h>

namespace srk::modules::inputs::direct_input {
	GamepadDriver::GamepadDriver(Input& input, LPDIRECTINPUTDEVICE8 dev) :
		_input(input),
		_dev(dev) {
		_dev->SetDataFormat(&c_dfDIJoystick);
		_dev->SetCooperativeLevel(input.getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	}

	GamepadDriver::~GamepadDriver() {
		_dev->Unacquire();
		_dev->Release();
	}

	size_t GamepadDriver::getInputLength() const {
		return sizeof(DIJOYSTATE) + 1;
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
		if (auto hr = _dev->Poll(); hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return false;
			if (FAILED(_dev->Poll())) return false;
		}

		auto raw = (uint8_t*)inputState;
		if (SUCCEEDED(_dev->GetDeviceState(sizeof(DIJOYSTATE), (DIJOYSTATE*)(raw + 1)))) {
			raw[0] = 1;

			return true;
		}

		return true;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace srk::enum_operators;

		float32_t val;
		if (auto raw = (const uint8_t*)inputState; raw[0]) {
			auto data = (const DIJOYSTATE*)(raw + 1);

			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= MAX_AXIS_KEY) {
				switch (cf.code) {
				case GamepadKeyCode::AXIS_1:
				case GamepadKeyCode::AXIS_1 + 1:
				case GamepadKeyCode::AXIS_1 + 2:
				case GamepadKeyCode::AXIS_1 + 3:
				case GamepadKeyCode::AXIS_1 + 4:
				case GamepadKeyCode::AXIS_1 + 5:
					val = DeviceStateValue((&data->lX)[(uint32_t)(cf.code - GamepadKeyCode::AXIS_1)]) / 65535.0f;
					break;
				case GamepadKeyCode::AXIS_1 + 6:
				case GamepadKeyCode::AXIS_1 + 7:
				{
					if (auto i = data->rgdwPOV[0]; i == std::numeric_limits<DWORD>::max()) {
						val = 0.5f;
					} else {
						auto a = Math::rad(DeviceStateValue(i) * Math::HUNDREDTH<DeviceStateValue>);
						val = 0.5f + (cf.code == GamepadKeyCode::AXIS_1 + 6 ? std::sin(a) : std::cos(a)) * 0.5f;
					}

					break;
				}
				default:
					val = defaultVal;
					break;
				}
				
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= MAX_BUTTON_KEY) {
				val = data->rgbButtons[(uint32_t)(cf.code - GamepadKeyCode::BUTTON_1)] & 0x80 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
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
			dst.set(GamepadVirtualKeyCode::L_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::L_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

			dst.set(GamepadVirtualKeyCode::R_STICK_X_LEFT, GamepadKeyCode::AXIS_1 + 3, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::R_STICK_X_RIGHT, GamepadKeyCode::AXIS_1 + 3, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::R_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 4, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::R_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 4, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

			dst.set(GamepadVirtualKeyCode::DPAD_LEFT, GamepadKeyCode::AXIS_1 + 6, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_RIGHT, GamepadKeyCode::AXIS_1 + 6, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::DPAD_DOWN, GamepadKeyCode::AXIS_1 + 7, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
			dst.set(GamepadVirtualKeyCode::DPAD_UP, GamepadKeyCode::AXIS_1 + 7, GamepadKeyFlag::HALF_BIG);

			dst.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 2, GamepadKeyFlag::HALF_BIG);
			dst.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 2, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

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
}