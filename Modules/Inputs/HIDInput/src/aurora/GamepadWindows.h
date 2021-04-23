#pragma once

#include "GamepadBase.h"

#if AE_OS == AE_OS_WIN
#include <hidsdi.h>

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Gamepad : public GamepadBase<64, 64, 64> {
	public:
		Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) override;

	protected:
		static constexpr size_t MAX_BUTTON = 32;

		using ButtonState = uint8_t[MAX_BUTTON];

		PHIDP_PREPARSED_DATA _preparsedData;
		std::vector<HIDP_BUTTON_CAPS> _buttonCaps;
		std::vector<HIDP_VALUE_CAPS> _valueCaps;
		std::unordered_map<uint32_t, void*> _inputCaps;

		mutable std::shared_mutex _inputStateMutex;
		std::unordered_map<uint32_t, uint16_t> _inputState;

		virtual void AE_CALL _doInput(bool dispatchEvent, InputBuffer& inputBuffer, size_t inputBufferSize) override;
		virtual bool AE_CALL _doOutput() override;
	};
}
#endif