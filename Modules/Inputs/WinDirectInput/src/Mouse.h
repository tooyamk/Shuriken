#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::win_direct_input {
	class AE_MODULE_DLL Mouse : public DeviceBase {
	public:
		Mouse(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);

		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		DIMOUSESTATE2 _state;

		POINT _pos;

		POINT AE_CALL _getClientPos() const;
	};
}