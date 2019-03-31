#pragma once

#include "Base.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL DeviceBase : public InputDevice {
	public:
		DeviceBase(LPDIRECTINPUTDEVICE8 dev, const InputDeviceGUID& guid, ui32 type);

		virtual const InputDeviceGUID& AE_CALL getGUID() const override;
		virtual ui32 AE_CALL getType() const override;

	protected:
		LPDIRECTINPUTDEVICE8 _dev;
		ui32 _type;
		InputDeviceGUID _guid;
	};
}