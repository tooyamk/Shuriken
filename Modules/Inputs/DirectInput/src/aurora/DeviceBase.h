#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::direct_input {
	class Input;

	class AE_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, LPDIRECTINPUTDEVICE8 dev, const InternalDeviceInfo& info);
		virtual ~DeviceBase();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;

	protected:
		IntrusivePtr<Input> _input;
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		InternalDeviceInfo _info;

		LPDIRECTINPUTDEVICE8 _dev;
	};
}