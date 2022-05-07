#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs::raw_input {
	class Input;

	class SRK_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, IApplication& app, const InternalDeviceInfo& info);
		virtual ~DeviceBase();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> SRK_CALL getEventDispatcher() override;
		virtual const DeviceInfo& SRK_CALL getInfo() const override;

	protected:
		IntrusivePtr<Input> _input;
		IntrusivePtr<IApplication> _app;
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		InternalDeviceInfo _info;

		IntrusivePtr<events::IEventListener<ApplicationEvent>> _rawIputHandler;

		void SRK_CALL _rawInputCallback(events::Event<ApplicationEvent>& e);

		virtual void SRK_CALL _rawInput(const RAWINPUT& rawInput) = 0;
	};
}