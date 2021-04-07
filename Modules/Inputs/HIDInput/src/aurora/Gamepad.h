#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(Input& input, const DeviceInfo& info);

		virtual uint32_t AE_CALL getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		virtual void AE_CALL _parse() override;
	};
}