#include "GamepadDriver.h"
#include "Input.h"

#include <wbemidl.h>
#include <oleauto.h>
#include <dinput.h>

namespace srk::modules::inputs::direct_input {
	GamepadDriver::GamepadDriver(Input& input, srk_IDirectInputDevice* dev, const DIDEVCAPS& caps) :
		_input(input),
		_dev(dev),
		_cpas(caps) {
		using namespace srk::enum_operators;
		_maxAxisKeyCode = GamepadKeyCode::AXIS_1 + _cpas.dwAxes - 1;
		_minDpadKeyCode = _maxAxisKeyCode + 1;
		_maxDpadKeyCode = _minDpadKeyCode + (_cpas.dwPOVs << 1) - 1;
		_maxButtonKeyCode = GamepadKeyCode::BUTTON_1 + _cpas.dwButtons - 1;

		_dev->SetDataFormat(&c_dfDIJoystick);
		_dev->SetCooperativeLevel(input.getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	}

	GamepadDriver::~GamepadDriver() {
		_dev->Unacquire();
		_dev->Release();
	}

	GamepadDriver* GamepadDriver::create(Input& input, srk_IDirectInputDevice* dev) {
		DIDEVCAPS caps;
		caps.dwSize = sizeof(caps);
		if (FAILED(dev->GetCapabilities(&caps))) return nullptr;

		return new GamepadDriver(input, dev, caps);
	}

	size_t GamepadDriver::getInputLength() const {
		return sizeof(DIJOYSTATE) + HEADER_LENGTH;
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
		if (SUCCEEDED(_dev->GetDeviceState(sizeof(DIJOYSTATE), (DIJOYSTATE*)(raw + HEADER_LENGTH)))) {
			raw[0] = 1;

			return true;
		}

		return true;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		using namespace srk::enum_operators;

		float32_t val;
		if (auto raw = (const uint8_t*)inputState; raw[0]) {
			auto data = (const DIJOYSTATE*)(raw + HEADER_LENGTH);

			if (cf.code >= GamepadKeyCode::AXIS_1 && cf.code <= _maxAxisKeyCode) {
				val = DeviceStateValue((&data->lX)[(uint32_t)(cf.code - GamepadKeyCode::AXIS_1)]) / 65535.0f;
			} else if (cf.code >= _minDpadKeyCode && cf.code <= _maxDpadKeyCode) {
				auto idx = (size_t)(cf.code - _minDpadKeyCode);
				if (auto i = data->rgdwPOV[idx >> 1]; i == std::numeric_limits<DWORD>::max()) {
					val = 0.5f;
				} else {
					auto a = Math::rad(DeviceStateValue(i) * Math::HUNDREDTH<DeviceStateValue>);
					val = 0.5f + ((idx & 0b1) == 0 ? std::sin(a) : std::cos(a)) * 0.5f;
				}
			} else if (cf.code >= GamepadKeyCode::BUTTON_1 && cf.code <= _maxButtonKeyCode) {
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
			dst.setDefault(_cpas.dwAxes, _cpas.dwPOVs, _cpas.dwButtons, false);
		}

		dst.undefinedCompletion(_cpas.dwAxes + (_cpas.dwPOVs << 1), _cpas.dwButtons);
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