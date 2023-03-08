#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs::direct_input {
	class Input;

	class SRK_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, srk_IDirectInputDevice* dev, const DeviceInfo& info);
		virtual ~DeviceBase();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> SRK_CALL getEventDispatcher() override;
		virtual const DeviceInfo& SRK_CALL getInfo() const override;

	protected:
		IntrusivePtr<Input> _input;
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;

		srk_IDirectInputDevice* _dev;
	};
}