#pragma once

#include "Input.h"

namespace aurora::modules::inputs::hid_input {
	class DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, const DeviceInfo& info, extensions::HIDDevice& hid);
		virtual ~DeviceBase();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;

	protected:
		IntrusivePtr<Input> _input;
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;
		extensions::HIDDevice* _hid;
	};
}