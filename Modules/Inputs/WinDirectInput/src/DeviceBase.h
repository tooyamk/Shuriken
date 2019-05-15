#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::inputs::win_direct_input {
	class Input;

	class AE_MODULE_DLL DeviceBase : public IInputDevice {
	public:
		DeviceBase(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);
		virtual ~DeviceBase();

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;
		virtual void AE_CALL setDeadZone(ui32 keyCode, f32 deadZone) override {}
		virtual void AE_CALL setVibration(f32 left, f32 right) override {}

	protected:
		RefPtr<Input> _input;
		events::EventDispatcher<DeviceEvent> _eventDispatcher;
		DeviceInfo _info;

		LPDIRECTINPUTDEVICE8 _dev;
		ui32 _type;
	};
}