#pragma once

#include "srk/modules/inputs/IInputModule.h"
#include "srk/IApplication.h"
#include <shared_mutex>

#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE(p) { if (p) { (p)->Release();  (p) = nullptr; } }
#endif

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL InternalDeviceInfo : public DeviceInfo {
	public:
		bool isXInput;
	};
}