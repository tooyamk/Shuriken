#pragma once

#include "Base.h"
#include "events/EventDispatcher.h"

namespace aurora::modules::win_direct_input {
	class AE_MODULE_DLL DirectInput : public IInputModule {
	public:
		DirectInput(Application* app);
		virtual ~DirectInput();

		virtual events::IEventDispatcher<InputModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IInputDevice* AE_CALL createDevice(const InputDeviceGUID& guid) override;

		HWND AE_CALL getHWND() const;

	private:
		Application* _app;

		LPDIRECTINPUT8 _di;

		std::vector<InputDeviceInfo> _devices;
		std::vector<InputDeviceInfo> _connectedDevices;
		std::vector<ui32> _keepDevices;
	
		events::EventDispatcher<InputModuleEvent> _eventDispatcher;

		static BOOL CALLBACK _enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext);
	};
}