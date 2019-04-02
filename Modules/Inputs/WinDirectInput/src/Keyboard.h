#pragma once

#include "DeviceBase.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL Keyboard : public DeviceBase {
	public:
		Keyboard(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info);

		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		using StateBuffer = ui8[256];

		StateBuffer _state;
	};
}