#pragma once

#include "DeviceBase.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL Gamepad : public DeviceBase {
	public:
		Gamepad(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info);

		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll() override;

	private:
		DIJOYSTATE2 _state;

		static f32 _transformStick(LONG value);
		static f32 _transformTrigger(LONG value);
		static f32 _transformAngle(DWORD value);
	};
}