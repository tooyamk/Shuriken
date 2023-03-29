#pragma once

#include "InputListener.h"

namespace srk::modules::inputs::raw_input {
	class Input;

	class SRK_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, windows::IWindow& win, const InternalDeviceInfo& info);
		virtual ~DeviceBase();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> SRK_CALL getEventDispatcher() override;
		virtual const DeviceInfo& SRK_CALL getInfo() const override;

	protected:
		InputListener _listener;

		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		InternalDeviceInfo _info;

		static void SRK_CALL _rawInputCallback(const RAWINPUT& rawInput, void* target);

		virtual void SRK_CALL _rawInput(const RAWINPUT& rawInput) = 0;
	};
}