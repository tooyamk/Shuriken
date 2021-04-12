#pragma once

#include "GamepadBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Gamepad : public GamepadBase<64, 64, 64> {
	public:
		Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override;

	protected:
		virtual void AE_CALL _doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) override;
		virtual bool AE_CALL _doOutput() override;
	};
}