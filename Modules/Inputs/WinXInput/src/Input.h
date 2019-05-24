#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::inputs::win_xinput {
	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, Application* app);
		virtual ~Input();

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const GUID& guid) override;

	private:
		RefPtr<Ref> _loader;
		RefPtr<Application> _app;

		std::vector<DeviceInfo> _devices;
		std::vector<DeviceInfo> _connectedDevices;
		std::vector<ui32> _keepDevices;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;
	};
}