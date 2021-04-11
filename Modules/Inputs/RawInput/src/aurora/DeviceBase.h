#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::raw_input {
	class Input;

	class AE_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, IApplication& app, const InternalDeviceInfo& info);
		virtual ~DeviceBase();

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;

	protected:
		IntrusivePtr<Input> _input;
		IntrusivePtr<IApplication> _app;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		InternalDeviceInfo _info;

		IntrusivePtr<events::IEventListener<ApplicationEvent>> _rawIputHandler;

		void AE_CALL _rawInputCallback(events::Event<ApplicationEvent>& e);

		virtual void AE_CALL _rawInput(const RAWINPUT& rawInput) = 0;
	};
}