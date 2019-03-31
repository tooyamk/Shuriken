#include "Keyboard.h"

namespace aurora::modules::win_direct_input {
	Keyboard::Keyboard(LPDIRECTINPUTDEVICE8 dev, const InputDeviceGUID& guid) : DeviceBase(dev, guid, InputDeviceType::KEYBOARD) {
		dev->SetDataFormat(&c_dfDIKeyboard);
		//dev->SetCooperativeLevel(hwndFound, DISCL_BACKGROUND | DISCL_EXCLUSIVE);
	}
}