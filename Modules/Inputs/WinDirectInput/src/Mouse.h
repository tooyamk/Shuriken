#pragma once

#include "DeviceBase.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info);

		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll() override;

	private:
		DIMOUSESTATE2 _state;
		DIMOUSESTATE2 _pollState;

		POINT _pos;

		POINT AE_CALL _getClientPos() const;
	};
}