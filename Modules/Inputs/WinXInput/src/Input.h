#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::win_xinput {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Application* app);
		virtual ~Input();

		virtual events::IEventDispatcher<InputModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const InputDeviceGUID& guid) override;

	private:
		RefPtr<Application> _app;

		std::vector<InputDeviceInfo> _devices;
		std::vector<InputDeviceInfo> _connectedDevices;
		std::vector<ui32> _keepDevices;

		events::EventDispatcher<InputModuleEvent> _eventDispatcher;
	};
}