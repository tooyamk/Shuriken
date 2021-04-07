#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);

		virtual uint32_t AE_CALL getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const override;

	protected:
		using StateBuffer = uint8_t[16];

		StateBuffer _state;
	};
}