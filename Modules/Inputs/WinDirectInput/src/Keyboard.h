#pragma once

#include "DeviceBase.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL Keyboard : public DeviceBase {
	public:
		Keyboard(LPDIRECTINPUTDEVICE8 dev, const InputDeviceGUID& guid);

	private:
	};
}