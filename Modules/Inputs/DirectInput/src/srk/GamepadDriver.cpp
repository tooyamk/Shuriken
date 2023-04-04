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

	std::optional<bool> GamepadDriver::readStateFromDevice(void* inputState) const {
		if (auto hr = _dev->Poll(); hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return std::nullopt;
			if (FAILED(_dev->Poll())) return std::nullopt;
		}

		auto raw = (uint8_t*)inputState;
		if (SUCCEEDED(_dev->GetDeviceState(sizeof(DIJOYSTATE), (DIJOYSTATE*)(raw + HEADER_LENGTH)))) {
			raw[0] = 1;

			return std::make_optional(true);
		}

		return std::nullopt;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCode keyCode) const {
		using namespace srk::enum_operators;

		if (!isStateReady(inputState)) return -1.0f;

		auto data = (const DIJOYSTATE*)((const uint8_t*)inputState + HEADER_LENGTH);

		if (keyCode >= GamepadKeyCode::AXIS_1 && keyCode <= _maxAxisKeyCode) {
			return DeviceStateValue((&data->lX)[(uint32_t)(keyCode- GamepadKeyCode::AXIS_1)]) / 65535.0f;
		} else if (keyCode >= _minDpadKeyCode && keyCode <= _maxDpadKeyCode) {
			auto idx = (size_t)(keyCode - _minDpadKeyCode);
			if (auto i = data->rgdwPOV[idx >> 1]; i == std::numeric_limits<DWORD>::max()) {
				return -1.0f;
			} else {
				auto a = Math::rad(DeviceStateValue(i) * Math::ONE_HUNDREDTH<DeviceStateValue>);
				return 0.5f + ((idx & 0b1) == 0 ? std::sin(a) : std::cos(a)) * 0.5f;
			}
		} else if (keyCode >= GamepadKeyCode::BUTTON_1 && keyCode <= _maxButtonKeyCode) {
			return data->rgbButtons[(uint32_t)(keyCode - GamepadKeyCode::BUTTON_1)] & 0x80 ? Math::ONE<DeviceStateValue> : Math::ZERO<DeviceStateValue>;
		} else {
			return -1.0f;
		}
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

		dst.undefinedCompletion<GamepadKeyCode::AXIS_1, GamepadKeyCode::AXIS_END, GamepadVirtualKeyCode::UNDEFINED_AXIS_1>(_cpas.dwAxes);
		dst.undefinedCompletion<GamepadKeyCode::HAT_1, GamepadKeyCode::HAT_END, GamepadVirtualKeyCode::UNDEFINED_HAT_1>(_cpas.dwPOVs);
		dst.undefinedCompletion<GamepadKeyCode::BUTTON_1, GamepadKeyCode::BUTTON_END, GamepadVirtualKeyCode::UNDEFINED_BUTTON_1>(_cpas.dwButtons);
	}
}