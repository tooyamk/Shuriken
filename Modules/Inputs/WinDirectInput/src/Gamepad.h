#pragma once

#include "DeviceBase.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info);

		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		DIJOYSTATE2 _state;

		static f32 _translateStick(LONG value);
		static f32 _translateTrigger(LONG value);
		static f32 _translateAngle(DWORD value);
	};
}