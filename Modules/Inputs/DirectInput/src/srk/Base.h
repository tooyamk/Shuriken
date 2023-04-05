#pragma once

#include "srk/modules/inputs/InputModule.h"

#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE(p) { if (p) { (p)->Release();  (p) = nullptr; } }
#endif

namespace srk::modules::inputs::direct_input {
	using srk_IDirectInput = IDirectInput8A;
	using srk_IDirectInputDevice = IDirectInputDevice8A;
	using srk_DIDEVICEINSTANCE = DIDEVICEINSTANCEA;

	class Input;
}