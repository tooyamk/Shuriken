#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::win_direct_input {
	class Input;

	class AE_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input* input, LPDIRECTINPUTDEVICE8 dev, const InputDeviceInfo& info);
		virtual ~DeviceBase();

		virtual events::IEventDispatcher<InputDeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const InputDeviceInfo& AE_CALL getInfo() const override;
		virtual void AE_CALL setDeadZone(ui32 keyCode, f32 deadZone) override {}

	protected:
		Input* _input;
		events::EventDispatcher<InputDeviceEvent> _eventDispatcher;
		InputDeviceInfo _info;

		LPDIRECTINPUTDEVICE8 _dev;
		ui32 _type;
	};
}