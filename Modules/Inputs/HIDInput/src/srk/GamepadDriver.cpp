#include "GamepadDriver.h"

#if SRK_OS != SRK_OS_WINDOWS
#include "Input.h"

namespace srk::modules::inputs::hid_input {
	GamepadDriver::GamepadDriver(Input& input, extensions::HIDDevice& hid) : GamepadDriverBase(input, hid) {
	}

	GamepadDriver::~GamepadDriver() {
	}

	size_t GamepadDriver::getInputLength() const {
		return 0;
	}

	size_t GamepadDriver::getOutputLength() const {
		return 0;
	}

	bool GamepadDriver::init(void* inputState, void* outputState) {
		return false;
	}

	bool GamepadDriver::isStateReady(const void* state) const {
		return false;
	}

	bool GamepadDriver::readStateFromDevice(void* inputState) const {
		return false;
	}

	float32_t GamepadDriver::readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const {
		return translate(defaultVal, cf.flags);
	}

	float32_t GamepadDriver::readDpadDataFromInputState(const void* inputState) const {
		return Math::NEGATIVE_ONE<DeviceStateValue>;
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
		if (src) {
			dst = *src;
		} else {
			dst.clear();
		}
	}
}
#endif