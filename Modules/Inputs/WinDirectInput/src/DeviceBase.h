#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::win_direct_input {
	class DirectInput;

	class AE_MODULE_DLL DeviceBase : public InputDevice {
	public:
		DeviceBase(DirectInput* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info);
		virtual ~DeviceBase();

		virtual events::IEventDispatcher<InputDeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const InputDeviceInfo& AE_CALL getInfo() const override;

	protected:
		DirectInput* _input;
		events::EventDispatcher<InputDeviceEvent> _eventDispatcher;
		InputDeviceInfo _info;

		LPDIRECTINPUTDEVICE8 _dev;
		ui32 _type;
	};
}